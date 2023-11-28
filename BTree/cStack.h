
template <typename T>
class cStack {
private:
    struct StackNode {
        T value;
        int position;
    };

    StackNode* stackArray;
    int top;
    int maxLength;

public:
    cStack(int maxLength) : top(-1), maxLength(maxLength) {
        stackArray = new StackNode[maxLength];
    }

    ~cStack() {
        delete[] stackArray;
    }

    void push(const T& value, int position) {
        if (isFull()) {
            throw std::overflow_error("Stack is full");
        }
        top++;
        stackArray[top].value = value;
        stackArray[top].position = position;
    }

    T pop(int& position) {
        if (isEmpty()) {
            throw std::underflow_error("Stack is empty");
        }

        position = stackArray[top].position;
        T value = stackArray[top].value;
        top--;
        return value;
    }

    T pop() {
        if (isEmpty()) {
            throw std::underflow_error("Stack is empty");
        }
        T value = stackArray[top].value;
        top--;
        return value;
    }

    T peek(int& position) {
        if (isEmpty()) {
            throw std::underflow_error("Stack is empty");
        }
        position = stackArray[top].position;
        return stackArray[top].value;
    }

    T peek() {
        if (isEmpty()) {
            throw std::underflow_error("Stack is empty");
        }
        return stackArray[top].value;
    }

    bool isEmpty() const {
        return top == -1;
    }

    bool isFull() const {
        return top == maxLength - 1;
    }
};
