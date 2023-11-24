
class cTreeMetadata {
private:
    int order; // The order of the B+ tree
    int rootPage; // The root page of the B+ tree
    int height; // The height of the B+ tree

public:
    // Constructor
    cTreeMetadata(int order, int rootPage, int height) {
        this->order = order;
        this->rootPage = rootPage;
        this->height = height;
    }

    // Getter methods
    int getOrder() const {
        return order;
    }

    int getRootPage() const {
        return rootPage;
    }

    int getHeight() const {
        return height;
    }

    // Setter methods
    void setOrder(int order) {
        this->order = order;
    }

    void setRootPage(int rootPage) {
        this->rootPage = rootPage;
    }

    void setHeight(int height) {
        this->height = height;
    }
};
