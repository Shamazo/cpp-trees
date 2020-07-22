#include "btree.h"
#include <stdexcept>
#include <numeric>
#include <algorithm>
#include "spdlog/spdlog.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iterator>
#include <optional>

/*******************
 * Helper functions
 *
 ******************/
template <typename T, typename Compare>
std::vector<std::size_t> sort_permutation(
        const std::vector<T>& vec,
        Compare compare)
{
    std::vector<std::size_t> p(vec.size());
    std::iota(p.begin(), p.end(), 0);
    std::sort(p.begin(), p.end(),
              [&](std::size_t i, std::size_t j){ return compare(vec[i], vec[j]); });
    return p;
}

template <typename T>
void apply_permutation_in_place(
        std::vector<T>& vec,
        const std::vector<std::size_t>& p)
{
    std::vector<bool> done(vec.size());
    for (std::size_t i = 0; i < vec.size(); ++i)
    {
        if (done[i]){
            continue;
        }
        done[i] = true;
        std::size_t prev_j = i;
        std::size_t j = p[i];
        while (i != j){
            std::swap(vec[prev_j], vec[j]);
            done[j] = true;
            prev_j = j;
            j = p[j];
        }
    }
}

template <typename T>
std::string array_to_string(T* arr, size_t size){
    std::ostringstream oss;
    if (size !=0){
        for (size_t i = 0; i < size; i++){
            oss << arr[i] << ", ";
        }
    }
    return oss.str();
}

void log_node(BNode* bnode){
    if (bnode->is_leaf){
        spdlog::info("META: is_leaf {}, num_elements: {} \n"
                     "    keys: {} \n"
                     "    vals: {}",
                     bnode->is_leaf,
                     bnode->num_elements,
                     array_to_string(bnode->keys, bnode->num_elements)),
                     array_to_string(bnode->children.vals, bnode->num_elements);
    } else {
        spdlog::info("META: is_leaf {}, num_elements: {} \n"
                     "    keys: {} \n"
                     "    pointers: {}",
                     bnode->is_leaf,
                     bnode->num_elements,
                     array_to_string(bnode->keys, bnode->num_elements),
                     array_to_string(bnode->children.child_pointers, bnode->num_elements));
    }

}


/***********************
 * Btree implementation
 *
 ***********************/

Btree::Btree() {
    this->root = nullptr;
    return;
}

/**
 * Loads the btree with given keys and values. Keys and values must have the same length
 * and do not need to be sorted.
 * TODO: template everything so we are not just dealing with ints.
 * @param keys, keys used to access the values
 * @param values, values associated with the keys
 */

void Btree::load(std::vector<int> keys, std::vector<int> values) {
//  sort based on keys
    auto p = sort_permutation(keys, [](int const& a, int const& b){return a < b;});
    apply_permutation_in_place(keys, p);
    apply_permutation_in_place(values, p);

    BNode* head_node = this->build_leaves(keys, values);
    log_node(head_node);
    size_t nodes_in_layer;
    while (true) {
        std::tie(head_node, nodes_in_layer) = build_layer(head_node);
        spdlog::info("Nodes in layer: {}", nodes_in_layer);
        if (nodes_in_layer == 0) {
            head_node->is_root = true;
            break;
        }
    }
    this->root = head_node;
}

/**
 * Build the leaves of a new Btree given keys and values.
 * @param keys Must be sorted and same length as values
 * @param values
 * @return pointer to the leftmost leaf
 */
BNode *Btree::build_leaves(std::vector<int> keys, std::vector<int> values) {
    if (keys.size() != values.size()){
        throw std::invalid_argument("keys and values must be the same length");
    }

    BNode* curr_node = new BNode;
    curr_node->previous = NULL;
    curr_node->is_leaf = true;
    curr_node->is_root = false;
    BNode* head = curr_node;

    size_t internal_idx = 0;
    size_t i = 0;
    while (i < keys.size()) {
        if (curr_node->num_elements > FAN_OUT){
            throw std::logic_error("Attempting to store more elements in node than fanout");
        }
        curr_node->keys[internal_idx] = keys[i];
        curr_node->children.vals[internal_idx] = values[i];
        curr_node->num_elements += 1;
        internal_idx += 1;

        if (internal_idx == FAN_OUT) {
            internal_idx = 0;
            BNode* next_node = new BNode;
            next_node->is_leaf = true;
            next_node->is_root = false;
            curr_node->next = next_node;
            curr_node = next_node;
            curr_node->next = NULL;
        }
        i++;
    }

    return head;
}

/**
 * Traverse the tree using BFS
 * @param func function applied to each BNode in the traversal.
 */
void Btree::traverse_tree(void (*func)(BNode *)) {
    size_t seen_nodes = 1;
    size_t curr_idx = 0;
    std::vector<BNode*> node_ptrs;
    node_ptrs.push_back(this->root);

    while (curr_idx < seen_nodes) {
        BNode* curr_node = node_ptrs[curr_idx];
        if (curr_node == nullptr) {
            curr_idx += 1;
            continue;
        }

        if (curr_node->is_leaf == false) {
            size_t ptrs_in_node = curr_node->num_elements + 1;  // +1 is the rightmost edge pointer
            for (size_t i = 0; i< ptrs_in_node; i ++){
                node_ptrs.push_back(curr_node->children.child_pointers[i]);
                seen_nodes += 1;
            }
        }
        curr_idx += 1;
        func(curr_node);
    }
    return;
}

/**
 * Get values for keys in range [lower, upper) not especially that upper is non inclusive
 * TODO: make upper an optional argument.
 * @param lower
 * @param upper
 * @return a std::vector containing the values which have keys in the provided range
 */
std::vector<int> Btree::get_vals(int lower, int upper) {
    std::vector<int> result;
    BNode* left_node = this->root;
    while (left_node->is_leaf == false) {
        std::optional<size_t> idx_opt = left_node->binary_search(0, left_node->num_elements, lower);
        if (!idx_opt.has_value()){
            spdlog::error("BNode binary search failed");
            throw std::runtime_error("BNode binary search failed");
        }
        size_t idx = idx_opt.value();
        while (idx > 0 && left_node->keys[idx] >= lower) {
            idx -= 1;
        }
        left_node = left_node->children.child_pointers[idx];
    }
    assert(left_node->is_leaf == true);

    std::optional<size_t> leaf_internal_idx_opt = left_node->binary_search(0, left_node->num_elements, lower);
    if (!leaf_internal_idx_opt.has_value()){
        spdlog::error("BNode binary search failed");
        throw std::runtime_error("BNode binary search failed");
    }
    size_t leaf_internal_idx = leaf_internal_idx_opt.value();
    // order matters here, needs to be >0 before we --
    while (leaf_internal_idx > 0 && left_node->keys[leaf_internal_idx - 1] >= lower) {
        leaf_internal_idx -= 1;
    }

    // if the lower bound is in the rightmost leaf and no values in the leaf match
    // we are done
    if (left_node->next == nullptr && left_node->keys[leaf_internal_idx] < lower) {
        return result;
    }

    while (left_node != nullptr) {
        for (size_t i = leaf_internal_idx; i < left_node->num_elements; i++) {
//            todo: would be nice to write this without branching.
            if ((left_node->keys[i] >= lower) & (left_node->keys[i] < upper)){
                result.push_back(left_node->children.vals[i]);
            }
        }
        if (left_node->keys[left_node->num_elements - 1] >= upper) {
            spdlog::debug("get vals breaking, {} >= {}",left_node->keys[left_node->num_elements - 1], upper);
            break;
        }
        spdlog::debug("get vals traversing right");
        left_node = left_node->next;
        leaf_internal_idx = 0;
    }
    return result;
}

/**
 * TODO: implement
 * @param bNode
 */
void Btree::split_node(BNode *bNode) {

}

std::pair<BNode *, size_t> Btree::build_layer(BNode* head_node) {
    BNode* curr_node = new BNode;
    BNode* first_node = curr_node;
    curr_node->previous = nullptr;
    curr_node->is_leaf = false;
    curr_node->is_root = false;
    size_t nodes_in_layer = 0;
    BNode* next_node;
    size_t i = 0;
    while (head_node != nullptr) {
        if(curr_node->num_elements > FAN_OUT){
            throw std::logic_error("Attempting to store more elements in node than fanout");
        }
        if (curr_node->num_elements == FAN_OUT - 1) {
            if (head_node != nullptr) {
                curr_node->children.child_pointers[FAN_OUT] = head_node->next;
            }

            next_node = new BNode;
            next_node->previous = curr_node;
            next_node->is_leaf = false;
            next_node->is_root = false;
            curr_node->next = next_node;
            curr_node = next_node;
            i = 0;
            nodes_in_layer += 1;
        }

        curr_node->children.child_pointers[i] = head_node;
        curr_node->keys[i] = head_node->keys[0];
        head_node = head_node->next;
        curr_node->num_elements += 1;
        i++;
    }

    return std::make_pair(first_node, nodes_in_layer);
}

/**
 * TODO: implement
 * @param key
 * @param value
 */
void Btree::insert(int key, int value) {

}


BNode::BNode() {
    this->previous = nullptr;
    this->next = nullptr;
    this->is_leaf = true;
    this->is_root = false;
    std::fill(this->children.vals, this->children.vals + FAN_OUT + 1, 0);
}


/**
 * A recursive binary search over the values in a BNode. Returns the position of the highest
 * lower bound. e.g for [2, 4, 6] a search for key 5 would return 1.
 * @param lk left key position to start searching from
 * @param rk right key position to end searching at
 * @param key the key in the BNode for which we are searching for a lower bound
 * @return the position of the lower bound, or nothing using std::optional
 */
std::optional<size_t> BNode::binary_search(size_t lk, size_t rk, int key) {
    if (lk <= rk) {
        size_t mid = lk + (rk - lk) / 2;
        spdlog::debug("mid {}", mid);
        if (this->keys[mid] == key) {
            return std::optional<size_t>{mid};
        } else if ((this->keys[mid - 1] <= key &&
                    (mid == rk))) {
            return std::optional<size_t>{mid - 1};
        } else if (this->keys[mid] > key) {
            if (mid == 0) {
                return std::optional<size_t>{mid};
            } else if (this->keys[mid - 1] < key) {
                spdlog::debug("mid minus one lt key");
                return std::optional<size_t>{mid - 1};
            }
            // recurse left
            return BNode::binary_search(lk, mid - 1, key);
        }
        // recurse right
        return BNode::binary_search(mid + 1, rk, key);
    }
    // lower bound not found
    return std::nullopt;
}
