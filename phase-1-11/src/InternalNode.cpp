#include "InternalNode.hpp"
#include "LeafNode.hpp"
#include "RecordPtr.hpp"

// creates internal node pointed to by tree_ptr or creates a new one
InternalNode::InternalNode(const TreePtr &tree_ptr) : TreeNode(INTERNAL, tree_ptr)
{
    this->keys.clear();
    this->tree_pointers.clear();
    if (!is_null(tree_ptr))
        this->load();
}

// max element from tree rooted at this node
Key InternalNode::max()
{
    Key max_key = DELETE_MARKER;
    TreeNode *last_tree_node = TreeNode::tree_node_factory(this->tree_pointers[this->size - 1]);
    max_key = last_tree_node->max();
    delete last_tree_node;
    return max_key;
}

// if internal node contains a single child, it is returned
TreePtr InternalNode::single_child_ptr()
{
    if (this->size == 1)
        return this->tree_pointers[0];
    return NULL_PTR;
}

// inserts <key, record_ptr> into subtree rooted at this node.
// returns pointer to split node if exists
// TODO: InternalNode::insert_key to be implemented
TreePtr InternalNode::insert_key(const Key &key, const RecordPtr &record_ptr)
{
    TreePtr new_tree_ptr = NULL_PTR;
    // cout << "InternalNode::insert_key not implemented" << endl;
    int size = this->keys.size();
    int childIndex = 0;
    TreePtr childPtr;
    while (true)
    {
        if (key <= this->keys[childIndex])
        {
            childPtr = this->tree_pointers[childIndex];
            break;
        }
        else if (childIndex + 1 < size)
        {
            childIndex++;
        }
        else
        {
            childPtr = this->tree_pointers[childIndex + 1];
            break;
        }
    }
    TreeNode *newChild = this->tree_node_factory(childPtr);
    TreePtr new_child = newChild->insert_key(key, record_ptr);
    if (new_child != NULL_PTR)
    {
        TreeNode *child = this->tree_node_factory(new_child);
        this->keys.push_back(newChild->max());
        // cout << "maxele" << newChild->max() << endl;
        this->tree_pointers.push_back(new_child);
        for (int a = this->keys.size() - 1; a >= 1; a--)
            if (this->keys[a] < this->keys[a - 1])
            {
                swap(this->keys[a], this->keys[a - 1]);
                swap(this->tree_pointers[a], this->tree_pointers[a + 1]);
            }
        this->size++;
        if (this->size > FANOUT)
        {
            InternalNode *temp = new InternalNode();
            for (int i = 0; i < this->keys.size(); i++)
            {
                temp->keys.push_back(this->keys[i]);
                temp->tree_pointers.push_back(this->tree_pointers[i]);
            }
            temp->tree_pointers.push_back(this->tree_pointers.back());
            // for(int i = 0; i < this->keys.size() ; i++){
            //     cout << this->keys[i] << endl;
            // }
            // cout << endl;
            this->tree_pointers.clear();
            this->keys.clear();
            this->size = 0;
            for (int i = 0; i < floor(FANOUT / 2); i++)
            {
                this->keys.push_back(temp->keys[i]);
            }
            for (int i = 0; i < floor(FANOUT / 2) + 1; i++)
            {
                this->tree_pointers.push_back(temp->tree_pointers[i]);
                this->size++;
            }
            InternalNode *newNode = new InternalNode();
            for (int i = floor(FANOUT / 2) + 1; i < temp->keys.size(); i++)
            {
                newNode->keys.push_back(temp->keys[i]);
            }
            for (int i = floor(FANOUT / 2) + 1; i < temp->tree_pointers.size(); i++)
            {
                newNode->tree_pointers.push_back(temp->tree_pointers[i]);
                newNode->size++;
            }
            // for(int i = 0; i < this->keys.size() ; i++){
            //     cout << this->keys[i] << endl;
            // }
            // cout << endl;
            // for(int i = 0; i < newNode->keys.size() ; i++){
            //     cout << newNode->keys[i] << endl;
            // }
            new_tree_ptr = newNode->tree_ptr;
            newNode->dump();
        }
    }
    this->dump();
    return new_tree_ptr;
}

// deletes key from subtree rooted at this if exists
// TODO: InternalNode::delete_key to be implemented
void InternalNode::delete_key(const Key &key)
{
    TreePtr new_tree_ptr = NULL_PTR;
    // cout << "internal Node delete is called" << endl;
    // FINDING THE CHILD WHERE THE KEY WOULD BE
    int pointerIndex = 0;
    for (int i = 0; i < this->keys.size(); i++)
    {
        if (key <= this->keys[i])
        {
            pointerIndex = i;
            break;
        }
        if (i == this->keys.size() - 1)
        {
            pointerIndex = i + 1;
        }
    }
    TreeNode *child = this->tree_node_factory(this->tree_pointers[pointerIndex]);
    child->delete_key(key);
    // checking Undeflow for the current pointer

    if (child->node_type == LEAF)
    {
        if (child->size < (FANOUT + 1) / 2)
        {
            bool dealt = 0;
            // left redistribute
            if (!dealt)
            {
                if (pointerIndex > 0)
                {
                    TreeNode *leftChild = this->tree_node_factory(this->tree_pointers[pointerIndex - 1]);
                    if (leftChild->size + child->size >= 2 * ((FANOUT + 1) / 2))
                    {
                        dealt = 1;
                        redistributeLeftLeaf(pointerIndex);
                    }
                }
            }
            // left merge
            if (!dealt)
            {
                if (pointerIndex > 0)
                {
                    TreeNode *leftChild = this->tree_node_factory(this->tree_pointers[pointerIndex - 1]);
                    if (leftChild->size + child->size <= FANOUT)
                    {
                        dealt = 1;
                        mergeLeftLeaf(pointerIndex);
                        // delete child;
                    }
                }
            }
            // right redistribute
            if (!dealt)
            {
                if (pointerIndex < this->size)
                {
                    TreeNode *rightChild = this->tree_node_factory(this->tree_pointers[pointerIndex + 1]);
                    if (rightChild->size + child->size >= 2 * ((FANOUT + 1) / 2))
                    {
                        dealt = 1;
                        redistributeRightLeaf(pointerIndex);
                    }
                }
            }
            // right merge
            if (!dealt)
            {
                if (pointerIndex < this->size)
                {
                    TreeNode *rightChild = this->tree_node_factory(this->tree_pointers[pointerIndex + 1]);
                    if (rightChild->size + child->size <= FANOUT)
                    {
                        dealt = 1;
                        mergeRightLeaf(pointerIndex);
                        // cout << "merged the right node" << endl;
                        // delete child;
                    }
                }
            }
        }
    }
    else if (child->node_type == INTERNAL)
    {
        // cout << "came here" << endl;
        if (child->size < (FANOUT + 1) / 2)
        {
            int dealt = 0;
            // left redistriute
            if (!dealt)
            {
                if (pointerIndex > 0)
                {
                    TreeNode *leftChild = this->tree_node_factory(this->tree_pointers[pointerIndex - 1]);
                    if (leftChild->size + child->size >= 2 * ((FANOUT + 1) / 2))
                    {
                        dealt = 1;
                        redistributeLeftInternal(pointerIndex);
                    }
                }
            }
            // left Merge
            if (!dealt)
            {
                if (pointerIndex > 0)
                {
                    TreeNode *leftChild = this->tree_node_factory(this->tree_pointers[pointerIndex - 1]);
                    if (leftChild->size + child->size <= FANOUT)
                    {
                        dealt = 1;
                        mergeLeftInternal(pointerIndex);
                    }
                }
            }
            // right redistribute
            if (!dealt)
            {
                if (pointerIndex < this->size)
                {
                    TreeNode *rightChild = this->tree_node_factory(this->tree_pointers[pointerIndex + 1]);
                    if (rightChild->size + child->size >= 2 * ((FANOUT + 1) / 2))
                    {
                        dealt = 1;
                        redistributeRightInternal(pointerIndex);
                    }
                }
            }
            // right merge
            if (!dealt)
            {
                if (pointerIndex < this->size)
                {
                    TreeNode *rightChild = this->tree_node_factory(this->tree_pointers[pointerIndex + 1]);
                    if (rightChild->size + child->size <= FANOUT)
                    {
                        dealt = 1;
                        mergeRightInternal(pointerIndex);
                        // cout << "merged the right node" << endl;
                        // delete child;
                    }
                }
            }
        }
    }
    this->dump();
}

void InternalNode::redistributeLeftLeaf(int index)
{
    LeafNode *child = new LeafNode(this->tree_pointers[index]);
    LeafNode *leftChild = new LeafNode(this->tree_pointers[index - 1]);
    // cout << leftChild->size << " " << (FANOUT+1)/2 << endl;
    for (int i = (FANOUT + 1) / 2; i < leftChild->size; i++)
    {
        Key hereKey = leftChild->data_pointers.rbegin()->first;
        RecordPtr herePtr = leftChild->data_pointers.rbegin()->second;
        leftChild->delete_key(hereKey);
        child->insert_key(hereKey, herePtr);
    }
    this->keys[index - 1] = leftChild->max();
    child->dump();
    leftChild->dump();
}

void InternalNode::mergeLeftLeaf(int index)
{
    // cout << index << endl;
    LeafNode *child = new LeafNode(this->tree_pointers[index]);
    LeafNode *leftChild = new LeafNode(this->tree_pointers[index - 1]);
    // cout << "merging the left node" << endl;
    // add the elements to the left child
    for (auto p : child->data_pointers)
    {
        leftChild->insert_key(p.first, p.second);
    }
    // delete the pointer for child and the entry in the internal Node
    vector<Key> newKeys;
    vector<TreePtr> newTreePointers;
    for (int i = 0; i < this->tree_pointers.size(); i++)
    {
        if (i != index)
        {
            newTreePointers.push_back(this->tree_pointers[i]);
        }
        if (i != index - 1)
        {
            newKeys.push_back(this->keys[i]);
        }
    }
    this->tree_pointers = newTreePointers;
    this->keys = newKeys;
    this->size = newTreePointers.size();
    // child->dump();
    leftChild->dump();
}

void InternalNode::redistributeRightLeaf(int index)
{
    LeafNode *child = new LeafNode(this->tree_pointers[index]);
    LeafNode *rightChild = new LeafNode(this->tree_pointers[index + 1]);
    // cout << leftChild->size << " " << (FANOUT+1)/2 << endl;
    for (int i = (FANOUT + 1) / 2; i < rightChild->size; i++)
    {
        Key hereKey = rightChild->data_pointers.begin()->first;
        RecordPtr herePtr = rightChild->data_pointers.begin()->second;
        rightChild->delete_key(hereKey);
        child->insert_key(hereKey, herePtr);
    }
    this->keys[index] = child->max();
    child->dump();
    rightChild->dump();
}

void InternalNode::mergeRightLeaf(int index)
{
    LeafNode *child = new LeafNode(this->tree_pointers[index]);
    LeafNode *rightChild = new LeafNode(this->tree_pointers[index + 1]);
    // cout << "merging the right node" << endl;
    // add the elements to the left child
    for (auto p : child->data_pointers)
    {
        rightChild->insert_key(p.first, p.second);
    }
    // delete the pointer for child and the entry in the internal Node
    vector<Key> newKeys;
    vector<TreePtr> newTreePointers;
    for (int i = 0; i < this->tree_pointers.size(); i++)
    {
        if (i != index)
        {
            newTreePointers.push_back(this->tree_pointers[i]);
            newKeys.push_back(this->keys[i]);
        }
    }
    this->tree_pointers = newTreePointers;
    this->keys = newKeys;
    this->size = newTreePointers.size();
    // child->dump();
    rightChild->dump();
}

void InternalNode::redistributeLeftInternal(int index)
{
    // cout << "left internal redistribute is called " << endl;
    InternalNode *child = new InternalNode(this->tree_pointers[index]);
    InternalNode *leftChild = new InternalNode(this->tree_pointers[index - 1]);
    InternalNode *temp = new InternalNode();
    for (int i = leftChild->size - 1; i >= (FANOUT + 1) / 2; i--)
    {
        temp->tree_pointers.push_back(leftChild->tree_pointers.back());
        leftChild->tree_pointers.pop_back();
        leftChild->size--;
    }
    for (int i = 0; i < child->size; i++)
    {
        temp->tree_pointers.push_back(child->tree_pointers[i]);
    }
    for (int i = leftChild->keys.size(); i >= (FANOUT + 1) / 2; i--)
    {
        swap(this->keys[index - 1], leftChild->keys[leftChild->keys.size() - 1]);
        temp->keys.push_back(leftChild->keys.back());
        leftChild->keys.pop_back();
    }
    for (int i = 0; i < child->keys.size(); i++)
    {
        temp->keys.push_back(child->keys[i]);
    }
    child->keys = temp->keys;
    child->tree_pointers = temp->tree_pointers;
    child->size = child->tree_pointers.size();
    child->dump();
    leftChild->dump();
}

void InternalNode::mergeLeftInternal(int index)
{
    // cout << "merging internal left node" << endl;
    InternalNode *child = new InternalNode(this->tree_pointers[index]);
    InternalNode *leftChild = new InternalNode(this->tree_pointers[index - 1]);
    for (int i = 0; i < child->tree_pointers.size(); i++)
    {
        leftChild->tree_pointers.push_back(child->tree_pointers[i]);
        leftChild->size++;
    }
    leftChild->keys.push_back(this->keys[index - 1]);
    for (int i = 0; i < child->keys.size(); i++)
    {
        leftChild->keys.push_back(child->keys[i]);
    }
    child->keys.clear();
    child->tree_pointers.clear();
    child->size = 0;
    InternalNode *temp = new InternalNode();
    for (int i = 0; i < this->keys.size(); i++)
    {
        if (i != index - 1)
        {
            temp->keys.push_back(this->keys[i]);
        }
    }
    for (int i = 0; i < this->tree_pointers.size(); i++)
    {
        if (i != index)
        {
            temp->tree_pointers.push_back(this->tree_pointers[i]);
            temp->size++;
        }
    }
    this->keys = temp->keys;
    this->tree_pointers = temp->tree_pointers;
    this->size = temp->size;
    // cout << this->size << " " << this->keys.size() << " " << this->tree_pointers.size() << endl;
    // cout << leftChild->size << " " << leftChild->keys.size() << " " << leftChild->tree_pointers.size() << endl;
    // for(int i = 0; i < this->keys.size() ; i++){
    //     cout << this->keys[i] << " ";
    // }
    // cout << endl;
    // for(int i = 0; i < leftChild->keys.size() ; i++){
    //     cout << leftChild->keys[i] << " ";
    // }
    // cout << endl;
    // cout << "merging done here" << endl;
    leftChild->dump();
    // child->dump();
}

void InternalNode::redistributeRightInternal(int index)
{
    InternalNode *child = new InternalNode(this->tree_pointers[index]);
    InternalNode *rightChild = new InternalNode(this->tree_pointers[index + 1]);
    InternalNode *temp = new InternalNode();
    // cout << index << endl;
    // cout << child->size << " " << child->keys.size() << " " << child->tree_pointers.size() << endl;
    int pointer1 = 0;
    while (child->size < (FANOUT + 1) / 2)
    {
        child->tree_pointers.push_back(rightChild->tree_pointers[pointer1]);
        pointer1++;
        child->size++;
    }
    int pointer2 = 0;
    while (child->keys.size() < ((FANOUT + 1) / 2) - 1)
    {
        swap(rightChild->keys[pointer2], this->keys[index]);
        child->keys.push_back(rightChild->keys[pointer2]);
        pointer2++;
    }
    for (int i = pointer1; i < rightChild->size; i++)
    {
        temp->tree_pointers.push_back(rightChild->tree_pointers[i]);
        temp->size++;
    }
    for (int i = pointer2; i < rightChild->keys.size(); i++)
    {
        temp->keys.push_back(rightChild->keys[i]);
    }
    rightChild->tree_pointers = temp->tree_pointers;
    rightChild->keys = temp->keys;
    rightChild->size = temp->size;
    // cout << rightChild->size << " " << rightChild->keys.size() << " " << rightChild->tree_pointers.size() << endl;
    // cout << child->size << " " << child->keys.size() << " " << child->tree_pointers.size() << endl;
    // for(int i = 0; i < rightChild->keys.size() ; i++){
    //     cout << rightChild->keys[i] << " ";
    // }
    // cout << endl;
    // for(int i = 0; i < child->keys.size() ; i++){
    //     cout << child->keys[i] << " ";
    // }
    // cout << endl;
    child->dump();
    rightChild->dump();
}

void InternalNode::mergeRightInternal(int index)
{
    InternalNode *child = new InternalNode(this->tree_pointers[index]);
    InternalNode *rightChild = new InternalNode(this->tree_pointers[index + 1]);

    for (int i = 0; i < rightChild->tree_pointers.size(); i++)
    {
        child->tree_pointers.push_back(rightChild->tree_pointers[i]);
        child->size++;
    }
    child->keys.push_back(this->keys[index]);
    for (int i = 0; i < rightChild->keys.size(); i++)
    {
        child->keys.push_back(rightChild->keys[i]);
    }
    rightChild->keys.clear();
    rightChild->tree_pointers.clear();
    rightChild->size = 0;
    InternalNode *temp = new InternalNode();
    for (int i = 0; i < this->keys.size(); i++)
    {
        if (i != index)
        {
            temp->keys.push_back(this->keys[i]);
        }
    }
    for (int i = 0; i < this->tree_pointers.size(); i++)
    {
        if (i != index + 1)
        {
            temp->tree_pointers.push_back(this->tree_pointers[i]);
            temp->size++;
        }
    }
    this->keys = temp->keys;
    this->tree_pointers = temp->tree_pointers;
    this->size = temp->size;

    child->dump();
}

// runs range query on subtree rooted at this node
void InternalNode::range(ostream &os, const Key &min_key, const Key &max_key) const
{
    BLOCK_ACCESSES++;
    for (int i = 0; i < this->size - 1; i++)
    {
        if (min_key <= this->keys[i])
        {
            auto *child_node = TreeNode::tree_node_factory(this->tree_pointers[i]);
            child_node->range(os, min_key, max_key);
            delete child_node;
            return;
        }
    }
    auto *child_node = TreeNode::tree_node_factory(this->tree_pointers[this->size - 1]);
    child_node->range(os, min_key, max_key);
    delete child_node;
}

// exports node - used for grading
void InternalNode::export_node(ostream &os)
{
    TreeNode::export_node(os);
    for (int i = 0; i < this->size - 1; i++)
        os << this->keys[i] << " ";
    os << endl;
    for (int i = 0; i < this->size; i++)
    {
        auto child_node = TreeNode::tree_node_factory(this->tree_pointers[i]);
        child_node->export_node(os);
        delete child_node;
    }
}

// writes subtree rooted at this node as a mermaid chart
void InternalNode::chart(ostream &os)
{
    string chart_node = this->tree_ptr + "[" + this->tree_ptr + BREAK;
    chart_node += "size: " + to_string(this->size) + BREAK;
    chart_node += "]";
    os << chart_node << endl;

    for (int i = 0; i < this->size; i++)
    {
        auto tree_node = TreeNode::tree_node_factory(this->tree_pointers[i]);
        tree_node->chart(os);
        delete tree_node;
        string link = this->tree_ptr + "-->|";

        if (i == 0)
            link += "x <= " + to_string(this->keys[i]);
        else if (i == this->size - 1)
        {
            link += to_string(this->keys[i - 1]) + " < x";
        }
        else
        {
            link += to_string(this->keys[i - 1]) + " < x <= " + to_string(this->keys[i]);
        }
        link += "|" + this->tree_pointers[i];
        os << link << endl;
    }
}

ostream &InternalNode::write(ostream &os) const
{
    TreeNode::write(os);
    for (int i = 0; i < this->size - 1; i++)
    {
        if (&os == &cout)
            os << "\nP" << i + 1 << ": ";
        os << this->tree_pointers[i] << " ";
        if (&os == &cout)
            os << "\nK" << i + 1 << ": ";
        os << this->keys[i] << " ";
    }
    if (&os == &cout)
        os << "\nP" << this->size << ": ";
    os << this->tree_pointers[this->size - 1];
    return os;
}

istream &InternalNode::read(istream &is)
{
    TreeNode::read(is);
    this->keys.assign(this->size - 1, DELETE_MARKER);
    this->tree_pointers.assign(this->size, NULL_PTR);
    for (int i = 0; i < this->size - 1; i++)
    {
        if (&is == &cin)
            cout << "P" << i + 1 << ": ";
        is >> this->tree_pointers[i];
        if (&is == &cin)
            cout << "K" << i + 1 << ": ";
        is >> this->keys[i];
    }
    if (&is == &cin)
        cout << "P" << this->size;
    is >> this->tree_pointers[this->size - 1];
    return is;
}
