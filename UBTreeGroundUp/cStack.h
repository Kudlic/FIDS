
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
    cStack(int maxLength);
    ~cStack();
    void push(const T& value, int position);
    T pop(int& position);
    T pop();
    T peek(int& position);
    T peek();
    bool isEmpty() const;
    bool isFull() const;
    int getOrder();
};

template <typename T>
cStack<T>::cStack(int maxLength) : top(-1), maxLength(maxLength) {
    stackArray = new StackNode[maxLength];
}

template <typename T>
cStack<T>::~cStack() {
    delete[] stackArray;
}

template <typename T>
void cStack<T>::push(const T& value, int position) {
    if (isFull()) {
        throw std::overflow_error("Stack is full");
    }
    top++;
    stackArray[top].value = value;
    stackArray[top].position = position;
}

template <typename T>
T cStack<T>::pop(int& position) {
    if (isEmpty()) {
        throw std::underflow_error("Stack is empty");
    }

    position = stackArray[top].position;
    T value = stackArray[top].value;
    top--;
    return value;
}

template <typename T>
T cStack<T>::pop() {
    if (isEmpty()) {
        throw std::underflow_error("Stack is empty");
    }
    T value = stackArray[top].value;
    top--;
    return value;
}

template <typename T>
T cStack<T>::peek(int& position) {
    if (isEmpty()) {
        throw std::underflow_error("Stack is empty");
    }
    position = stackArray[top].position;
    return stackArray[top].value;
}

template <typename T>
T cStack<T>::peek() {
    if (isEmpty()) {
        throw std::underflow_error("Stack is empty");
    }
    return stackArray[top].value;
}

template <typename T>
bool cStack<T>::isEmpty() const {
    return top == -1;
}

template <typename T>
bool cStack<T>::isFull() const {
    return top == maxLength - 1;
}

template <typename T>
int cStack<T>::getOrder() {
    return top;
}

