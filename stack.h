#pragma once

template <class item_t, int num_items = 64>
class Stack {
public:
    Stack() {
        clear();
    }

    int size() {
        return mItemCount;
    }

    void clear() {
        mTop = 0;
        mItemCount = 0;
    }

    void push(item_t item) {
        if (mItemCount < num_items) {
            mData[mTop] = item;
            ++mTop;
            ++mItemCount;
        }
        // Note: In a real implementation, you might want to handle stack overflow
    }

    item_t pop() {
        if (mItemCount > 0) {
            --mTop;
            --mItemCount;
            return mData[mTop];
        }
        // Note: In a real implementation, you might want to handle stack underflow
        return item_t(); // Returns a default-constructed item if stack is empty
    }

    item_t peek() {
        if (mItemCount > 0) {
            return mData[mTop - 1];
        }
        // Note: In a real implementation, you might want to handle empty stack
        return item_t(); // Returns a default-constructed item if stack is empty
    }

    bool isEmpty() {
        return mItemCount == 0;
    }

    bool isFull() {
        return mItemCount == num_items;
    }

protected:
    item_t mData[num_items];
    int mTop = 0;
    int mItemCount = 0;

private:
    // Prevent use of the copy constructor
    Stack(const Stack<item_t, num_items>& rhs) {}
};