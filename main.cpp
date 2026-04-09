#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include <cmath> // For std::clamp
#include <cfloat> // For FLT_MAX

using namespace sf;
using namespace std;

// --- Configuration ---
const int WINDOW_WIDTH = 1200; 
const int WINDOW_HEIGHT = 800; 

// --- UI / Button Function ---
bool drawButton(RenderWindow& window, const Font& font, const string& label, Vector2f pos, Vector2f size = {120, 40}) {
    RectangleShape rect(size);
    Vector2i mouse = Mouse::getPosition(window);
    bool hovered = rect.getGlobalBounds().contains(Vector2f(mouse));
    rect.setPosition(pos);
    rect.setFillColor(hovered ? Color(100, 100, 255) : Color(80, 80, 80));

    Text text(font, label);
    text.setCharacterSize(18);
    text.setFillColor(Color::White);
    
    FloatRect textBounds = text.getLocalBounds();
    text.setPosition(pos + Vector2f(
        (size.x - textBounds.size.x) / 2 - textBounds.position.x, 
        (size.y - textBounds.size.y) / 2 - textBounds.position.y
    ));

    window.draw(rect);
    window.draw(text);
    return hovered;
}

// --- Global UI Draw Function (for animations) ---
void drawGlobalUI(RenderWindow& window, const Font& font, int animationDelay) {
    drawButton(window, font, "Reset", {850, 10}, {120, 40});
    drawButton(window, font, "New Array", {980, 10}, {120, 40});
    drawButton(window, font, "Speed -", {850, 60}, {80, 30});
    drawButton(window, font, "Speed +", {940, 60}, {80, 30});
    Text speedText(font, "Delay: " + to_string(animationDelay) + "ms");
    speedText.setCharacterSize(16);
    speedText.setFillColor(Color::White);
    speedText.setPosition({1030, 65});
    window.draw(speedText);
}

// --- Event Polling & UI Check ---
struct UIState {
    bool resetPressed = false;
    bool newArrayPressed = false;
    bool shouldClose = false;
};

// ✅ NEW: Added viewOffsetX, min/maxTreeX for panning
UIState pollEventsAndCheckUI(
    RenderWindow& window, const Font& font, int& animationDelay, 
    float& viewOffsetX, float minTreeX, float maxTreeX,
    float& viewOffsetY, float maxTreeY,
    bool& isPanning, Vector2i& lastPanMousePos
) {
    UIState state;
    bool speedUpPressed = false;
    bool speedDownPressed = false;
    
    while (auto event = window.pollEvent()) {
        if (event->is<Event::Closed>()) {
            state.shouldClose = true;
            window.close();
        }

        // Handle scroll wheel (Vertical)
        if (auto scrollEvent = event->getIf<Event::MouseWheelScrolled>()) {
            if (scrollEvent->wheel == Mouse::Wheel::Vertical) {
                float delta = scrollEvent->delta; 
                viewOffsetY = std::clamp(viewOffsetY - delta * 20.0f, 0.0f, std::max(0.0f, maxTreeY - WINDOW_HEIGHT + 100.0f));
            }
        }

        // ✅ NEW: Handle Mouse Panning (Horizontal)
        if (auto mbp = event->getIf<Event::MouseButtonPressed>()) {
            if (mbp->button == Mouse::Button::Left) {
                isPanning = true;
                lastPanMousePos = Mouse::getPosition(window);
            }
        }
        if (auto mbr = event->getIf<Event::MouseButtonReleased>()) {
            if (mbr->button == Mouse::Button::Left) {
                isPanning = false;
            }
        }
        if (event->is<Event::MouseMoved>()) {
            if (isPanning) {
                Vector2i currentMousePos = Mouse::getPosition(window);
                Vector2i delta = currentMousePos - lastPanMousePos;
                
                // Panning logic
                viewOffsetX -= delta.x;
                lastPanMousePos = currentMousePos;

                // Clamp the horizontal view
                float minPan = minTreeX - 50; // Allow 50px of overpan
                float maxPan = maxTreeX - WINDOW_WIDTH + 50;
                viewOffsetX = std::clamp(viewOffsetX, std::min(minPan, maxPan), std::max(minPan, maxPan));
            }
        }
    }

    // --- Check UI Buttons (this part is unchanged) ---
    Vector2i mouse = Mouse::getPosition(window);
    bool mousePressed = Mouse::isButtonPressed(Mouse::Button::Left);

    RectangleShape r_reset({120, 40}); r_reset.setPosition({850, 10});
    RectangleShape r_new({120, 40}); r_new.setPosition({980, 10});
    RectangleShape r_speedDown({80, 30}); r_speedDown.setPosition({850, 60});
    RectangleShape r_speedUp({80, 30}); r_speedUp.setPosition({940, 60});

    if (mousePressed && r_reset.getGlobalBounds().contains(Vector2f(mouse))) state.resetPressed = true;
    if (mousePressed && r_new.getGlobalBounds().contains(Vector2f(mouse))) state.newArrayPressed = true;
    if (mousePressed && r_speedDown.getGlobalBounds().contains(Vector2f(mouse))) speedDownPressed = true;
    if (mousePressed && r_speedUp.getGlobalBounds().contains(Vector2f(mouse))) speedUpPressed = true;

    if (speedDownPressed) { 
        animationDelay = min(1000, animationDelay + 50);
        sleep(milliseconds(100)); 
    }
    if (speedUpPressed) { 
        animationDelay = max(0, animationDelay - 50);
        sleep(milliseconds(100)); 
    }
    
    return state;
}


// ##################################################################
// --- MODE 1: BAR VISUALIZATION (Bubble, Insertion, Selection) ---
// ##################################################################

// Note: This whole section is unchanged, but pollEventsAndCheckUI now needs dummy view params
void drawSortState_bars(RenderWindow& window, const Font& font, const vector<int>& arr,
                        int& animationDelay, const string& title,
                        const map<int, Color>& highlights = {}) {
    window.clear(Color::Black);

    if (!title.empty()) {
        Text titleText(font, title);
        titleText.setCharacterSize(24);
        titleText.setFillColor(Color::White);
        titleText.setPosition({(WINDOW_WIDTH - titleText.getLocalBounds().size.x) / 2, 100});
        window.draw(titleText);
    }
    
    float barWidth = (float)WINDOW_WIDTH / arr.size();
    for (size_t i = 0; i < arr.size(); i++) {
        RectangleShape bar({barWidth - 2, (float)arr[i]});
        bar.setPosition({i * barWidth, (float)(WINDOW_HEIGHT - arr[i])});

        if (highlights.count(i)) {
            bar.setFillColor(highlights.at(i));
        } else {
            bar.setFillColor(Color::Green);
        }
        window.draw(bar);

        if (barWidth > 20) { 
            Text valText(font, to_string(arr[i]));
            valText.setCharacterSize(14);
            valText.setFillColor(Color::White);
            FloatRect textBounds = valText.getLocalBounds();
            valText.setPosition({
                (i * barWidth + (barWidth / 2)) - (textBounds.size.x / 2) - textBounds.position.x,
                (float)(WINDOW_HEIGHT - arr[i] - 20)
            });
            window.draw(valText);
        }
    }
    
    drawGlobalUI(window, font, animationDelay);
}

// Bar sort functions just need to pass dummy view variables to pollEvents
bool bubbleSort_bars(RenderWindow& window, const Font& font, vector<int>& arr, int& animationDelay) {
    string title = "Bubble Sort";
    float dummyViewX=0, dummyMinX=0, dummyMaxX=0, dummyViewY=0, dummyMaxY=0;
    bool dummyPan = false; Vector2i dummyMouse;
    for (size_t i = 0; i < arr.size(); i++) {
        for (size_t j = 0; j < arr.size() - i - 1; j++) {
            UIState state = pollEventsAndCheckUI(window, font, animationDelay, dummyViewX, dummyMinX, dummyMaxX, dummyViewY, dummyMaxY, dummyPan, dummyMouse);
            if (state.shouldClose || state.resetPressed || state.newArrayPressed) return false; 
            
            drawSortState_bars(window, font, arr, animationDelay, title, {{j, Color::Yellow}, {j + 1, Color::Yellow}});
            window.display();
            sleep(milliseconds(animationDelay));

            if (arr[j] > arr[j + 1]) {
                swap(arr[j], arr[j + 1]);
                drawSortState_bars(window, font, arr, animationDelay, title, {{j, Color::Red}, {j + 1, Color::Red}});
                window.display();
                sleep(milliseconds(animationDelay));
            }
        }
    }
    return true; 
}

bool insertionSort_bars(RenderWindow& window, const Font& font, vector<int>& arr, int& animationDelay) {
    string title = "Insertion Sort";
    float dummyViewX=0, dummyMinX=0, dummyMaxX=0, dummyViewY=0, dummyMaxY=0;
    bool dummyPan = false; Vector2i dummyMouse;
    for (size_t i = 1; i < arr.size(); i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            UIState state = pollEventsAndCheckUI(window, font, animationDelay, dummyViewX, dummyMinX, dummyMaxX, dummyViewY, dummyMaxY, dummyPan, dummyMouse);
            if (state.shouldClose || state.resetPressed || state.newArrayPressed) return false;
            drawSortState_bars(window, font, arr, animationDelay, title, {{j, Color::Yellow}, {j + 1, Color::Yellow}});
            window.display();
            sleep(milliseconds(animationDelay));
            arr[j + 1] = arr[j];
            j = j - 1;
            drawSortState_bars(window, font, arr, animationDelay, title, {{j + 1, Color::Red}, {j + 2, Color::Red}});
            window.display();
            sleep(milliseconds(animationDelay));
        }
        arr[j + 1] = key;
    }
    return true;
}

bool selectionSort_bars(RenderWindow& window, const Font& font, vector<int>& arr, int& animationDelay) {
    string title = "Selection Sort";
    float dummyViewX=0, dummyMinX=0, dummyMaxX=0, dummyViewY=0, dummyMaxY=0;
    bool dummyPan = false; Vector2i dummyMouse;
    for (size_t i = 0; i < arr.size() - 1; i++) {
        int min_idx = i;
        for (size_t j = i + 1; j < arr.size(); j++) {
            UIState state = pollEventsAndCheckUI(window, font, animationDelay, dummyViewX, dummyMinX, dummyMaxX, dummyViewY, dummyMaxY, dummyPan, dummyMouse);
            if (state.shouldClose || state.resetPressed || state.newArrayPressed) return false;
            drawSortState_bars(window, font, arr, animationDelay, title, {{min_idx, Color::Yellow}, {j, Color::Yellow}});
            window.display();
            sleep(milliseconds(animationDelay));
            if (arr[j] < arr[min_idx]) {
                min_idx = j;
            }
        }
        swap(arr[min_idx], arr[i]);
        drawSortState_bars(window, font, arr, animationDelay, title, {{min_idx, Color::Red}, {i, Color::Red}});
        window.display();
        sleep(milliseconds(animationDelay));
    }
    return true;
}


// ################################################################
// --- MODE 2: TREE VISUALIZATION (Quick, Merge) ---
// ################################################################

struct Node {
    vector<int> data;
    string title;
    FloatRect bounds; 
    bool isSorted = false;
    bool isActive = false;
    Node *left = nullptr, *right = nullptr, *parent = nullptr;
    ~Node() { delete left; delete right; }
};

Vector2f getNodeSize(const vector<int>& data) {
    float boxSize = 30;
    float spacing = 5;
    float totalWidth = data.size() * (boxSize + spacing) - spacing;
    return {totalWidth + 40, 80}; 
}

// ✅ NEW: Added minTreeX and maxTreeX to track horizontal bounds
void calculateTreeLayout(Node* node, int x, int y, int h_spacing, int v_spacing, 
                         float& minTreeX, float& maxTreeX, float& maxTreeY) {
    if (!node) return;
    Vector2f size = getNodeSize(node->data);
    node->bounds.position = {x - size.x / 2, (float)y};
    node->bounds.size = size;
    
    maxTreeY = std::max(maxTreeY, (float)y + node->bounds.size.y);
    minTreeX = std::min(minTreeX, node->bounds.position.x);
    maxTreeX = std::max(maxTreeX, node->bounds.position.x + node->bounds.size.x);

    if (node->left) {
        node->left->parent = node;
        calculateTreeLayout(node->left, x - h_spacing, y + v_spacing, h_spacing / 2, v_spacing, minTreeX, maxTreeX, maxTreeY);
    }
    if (node->right) {
        node->right->parent = node;
        calculateTreeLayout(node->right, x + h_spacing, y + v_spacing, h_spacing / 2, v_spacing, minTreeX, maxTreeX, maxTreeY);
    }
}

// ✅ NEW: Added viewOffsetX for panning
void drawNodeBoxes(RenderWindow& window, const Font& font, const Node* node, 
                   float viewOffsetX, float viewOffsetY, const map<int, Color>& highlights = {}) {
    if (node->data.empty()) return;

    float boxSize = 30; 
    float spacing = 5;
    float totalWidth = node->data.size() * (boxSize + spacing) - spacing;
    
    // Apply horizontal pan offset
    float startX = node->bounds.position.x + (node->bounds.size.x - totalWidth) / 2 - viewOffsetX;
    float startY = node->bounds.position.y + (node->bounds.size.y - boxSize) / 2 - viewOffsetY; 

    for (size_t i = 0; i < node->data.size(); i++) {
        RectangleShape box({boxSize, boxSize});
        box.setPosition({startX + i * (boxSize + spacing), startY});
        
        if (highlights.count(i)) { box.setFillColor(highlights.at(i)); }
        else { box.setFillColor(Color(50, 50, 150)); }
        box.setOutlineColor(node->isSorted ? Color::Green : Color(200, 200, 200));
        box.setOutlineThickness(1);
        window.draw(box);

        Text valText(font, to_string(node->data[i]));
        valText.setCharacterSize(16);
        valText.setFillColor(Color::White);
        FloatRect textBounds = valText.getLocalBounds();
        
        valText.setPosition({
            box.getPosition().x + (boxSize - textBounds.size.x) / 2 - textBounds.position.x,
            box.getPosition().y + (boxSize - textBounds.size.y) / 2 - textBounds.position.y - 2
        });
        window.draw(valText);
    }
}

// ✅ NEW: Added viewOffsetX for panning
void drawTree(RenderWindow& window, const Font& font, Node* node, float viewOffsetX, float viewOffsetY) {
    if (!node) return;

    // Apply scroll and pan offsets to all coordinates
    if (node->left) {
        Vertex line[2];
        line[0].position = node->bounds.position + Vector2f(node->bounds.size.x / 2, node->bounds.size.y);
        line[0].position.x -= viewOffsetX; line[0].position.y -= viewOffsetY;
        line[0].color = Color::White;
        line[1].position = node->left->bounds.position + Vector2f(node->left->bounds.size.x / 2, 0);
        line[1].position.x -= viewOffsetX; line[1].position.y -= viewOffsetY;
        line[1].color = Color::White;
        window.draw(line, 2, sf::PrimitiveType::Lines);
    }
    if (node->right) {
        Vertex line[2];
        line[0].position = node->bounds.position + Vector2f(node->bounds.size.x / 2, node->bounds.size.y);
        line[0].position.x -= viewOffsetX; line[0].position.y -= viewOffsetY;
        line[0].color = Color::White;
        line[1].position = node->right->bounds.position + Vector2f(node->right->bounds.size.x / 2, 0);
        line[1].position.x -= viewOffsetX; line[1].position.y -= viewOffsetY;
        line[1].color = Color::White;
        window.draw(line, 2, sf::PrimitiveType::Lines);
    }

    RectangleShape rect(node->bounds.size);
    rect.setPosition({node->bounds.position.x - viewOffsetX, node->bounds.position.y - viewOffsetY});
    rect.setFillColor(Color(30, 30, 30));
    rect.setOutlineThickness(2);
    rect.setOutlineColor(node->isActive ? Color::Blue : (node->isSorted ? Color::Green : Color(80, 80, 80)));
    window.draw(rect);

    Text titleText(font, node->title);
    titleText.setCharacterSize(12);
    titleText.setFillColor(Color::White);
    titleText.setPosition({node->bounds.position.x + 5 - viewOffsetX, node->bounds.position.y + 5 - viewOffsetY});
    window.draw(titleText);
    
    drawNodeBoxes(window, font, node, viewOffsetX, viewOffsetY);
    
    drawTree(window, font, node->left, viewOffsetX, viewOffsetY);
    drawTree(window, font, node->right, viewOffsetX, viewOffsetY);
}

// ✅ NEW: Added viewOffsetX for panning
void drawTreeState(RenderWindow& window, const Font& font, Node* root, Node* activeNode,
                   int& animationDelay, float viewOffsetX, float viewOffsetY, const map<int, Color>& highlights = {}) {
    window.clear(Color::Black);
    
    drawTree(window, font, root, viewOffsetX, viewOffsetY);
    
    if (activeNode) {
        float boxSize = 30;
        RectangleShape clearer({activeNode->bounds.size.x - 10, boxSize + 10}); 
        clearer.setPosition({
            activeNode->bounds.position.x + 5 - viewOffsetX, 
            activeNode->bounds.position.y + (activeNode->bounds.size.y - boxSize) / 2 - 5 - viewOffsetY
        });
        clearer.setFillColor(Color(30, 30, 30));
        window.draw(clearer);
        
        drawNodeBoxes(window, font, activeNode, viewOffsetX, viewOffsetY, highlights);
    }

    drawGlobalUI(window, font, animationDelay);
    window.display();
}

// --- Merge Sort (Tree) ---
// ✅ NEW: This function is completely rewritten for real-time division
bool animateMergeSort(
    RenderWindow& window, const Font& font, Node* node, int& animationDelay, Node* root, 
    float& viewOffsetX, float& minTreeX, float& maxTreeX,
    float& viewOffsetY, float& maxTreeY,
    bool& isPanning, Vector2i& lastPanMousePos
) {
    if (!node || node->data.size() <= 1) {
        if(node) node->isSorted = true;
        return true;
    }

    // --- 1. Divide Step ---
    node->isActive = true;
    node->title = "Splitting";
    
    UIState state = pollEventsAndCheckUI(window, font, animationDelay, viewOffsetX, minTreeX, maxTreeX, viewOffsetY, maxTreeY, isPanning, lastPanMousePos);
    if (state.shouldClose || state.resetPressed || state.newArrayPressed) return false;
    drawTreeState(window, font, root, node, animationDelay, viewOffsetX, viewOffsetY, {});
    sleep(milliseconds(animationDelay * 2 + 50)); // Pause to show split

    int m = node->data.size() / 2;
    node->left = new Node();
    node->left->data = vector<int>(node->data.begin(), node->data.begin() + m);
    node->right = new Node();
    node->right->data = vector<int>(node->data.begin() + m, node->data.end());

    // Recalculate layout with new children
    minTreeX = FLT_MAX; maxTreeX = -FLT_MAX; maxTreeY = 0;
    calculateTreeLayout(root, WINDOW_WIDTH / 2, 100, WINDOW_WIDTH / 4, 120, minTreeX, maxTreeX, maxTreeY);
    
    state = pollEventsAndCheckUI(window, font, animationDelay, viewOffsetX, minTreeX, maxTreeX, viewOffsetY, maxTreeY, isPanning, lastPanMousePos);
    if (state.shouldClose || state.resetPressed || state.newArrayPressed) return false;
    drawTreeState(window, font, root, node, animationDelay, viewOffsetX, viewOffsetY, {});
    sleep(milliseconds(animationDelay * 2 + 50));
    
    node->isActive = false;

    // --- 2. Recurse Step ---
    if (!animateMergeSort(window, font, node->left, animationDelay, root, viewOffsetX, minTreeX, maxTreeX, viewOffsetY, maxTreeY, isPanning, lastPanMousePos)) return false;
    if (!animateMergeSort(window, font, node->right, animationDelay, root, viewOffsetX, minTreeX, maxTreeX, viewOffsetY, maxTreeY, isPanning, lastPanMousePos)) return false;

    // --- 3. Merge Step ---
    node->isActive = true;
    node->title = "Merging";
    
    vector<int> L = node->left->data;
    vector<int> R = node->right->data;
    int n1 = L.size(); int n2 = R.size();
    int i = 0, j = 0, k = 0;
    
    while (i < n1 && j < n2) {
        state = pollEventsAndCheckUI(window, font, animationDelay, viewOffsetX, minTreeX, maxTreeX, viewOffsetY, maxTreeY, isPanning, lastPanMousePos);
        if (state.shouldClose || state.resetPressed || state.newArrayPressed) return false;

        node->left->isActive = true; node->right->isActive = true;
        drawTreeState(window, font, root, node->left, animationDelay, viewOffsetX, viewOffsetY, {{i, Color::Yellow}});
        drawTreeState(window, font, root, node->right, animationDelay, viewOffsetX, viewOffsetY, {{j, Color::Yellow}});
        
        map<int, Color> mergeHighlights;
        for(int m=0; m < k; m++) mergeHighlights[m] = Color::Green; 
        
        drawTreeState(window, font, root, node, animationDelay, viewOffsetX, viewOffsetY, mergeHighlights);
        sleep(milliseconds(animationDelay));

        if (L[i] <= R[j]) { node->data[k] = L[i]; i++; }
        else { node->data[k] = R[j]; j++; }
        k++;
        
        mergeHighlights[k-1] = Color::Red;
        drawTreeState(window, font, root, node, animationDelay, viewOffsetX, viewOffsetY, mergeHighlights);
        sleep(milliseconds(animationDelay));

        node->left->isActive = false; node->right->isActive = false;
    }

    while (i < n1) {
        state = pollEventsAndCheckUI(window, font, animationDelay, viewOffsetX, minTreeX, maxTreeX, viewOffsetY, maxTreeY, isPanning, lastPanMousePos);
        if (state.shouldClose || state.resetPressed || state.newArrayPressed) return false;
        node->data[k] = L[i]; 
        map<int, Color> mergeHighlights;
        for(int m=0; m < k; m++) mergeHighlights[m] = Color::Green;
        mergeHighlights[k] = Color::Red;
        drawTreeState(window, font, root, node, animationDelay, viewOffsetX, viewOffsetY, mergeHighlights);
        sleep(milliseconds(animationDelay));
        i++; k++;
    }
    while (j < n2) {
        state = pollEventsAndCheckUI(window, font, animationDelay, viewOffsetX, minTreeX, maxTreeX, viewOffsetY, maxTreeY, isPanning, lastPanMousePos);
        if (state.shouldClose || state.resetPressed || state.newArrayPressed) return false;
        node->data[k] = R[j]; 
        map<int, Color> mergeHighlights;
        for(int m=0; m < k; m++) mergeHighlights[m] = Color::Green;
        mergeHighlights[k] = Color::Red;
        drawTreeState(window, font, root, node, animationDelay, viewOffsetX, viewOffsetY, mergeHighlights);
        sleep(milliseconds(animationDelay));
        j++; k++;
    }

    node->isSorted = true;
    node->isActive = false;
    drawTreeState(window, font, root, node, animationDelay, viewOffsetX, viewOffsetY, {});
    sleep(milliseconds(animationDelay));
    
    return true;
}

// --- Quick Sort (Tree) ---
// ✅ NEW: Added all pan/scroll parameters to signature
// --- Quick Sort (Tree) ---
// ✅ FIX: Added all pan/scroll parameters and proper "combine" animation
bool animateQuickSort(
    RenderWindow& window, const Font& font, Node* node, int& animationDelay, Node* root, 
    float& viewOffsetX, float& minTreeX, float& maxTreeX,
    float& viewOffsetY, float& maxTreeY,
    bool& isPanning, Vector2i& lastPanMousePos
) {
    if (!node || node->data.size() <= 1) {
        if (node) node->isSorted = true;
        return true;
    }
    
    node->isActive = true;
    node->title = "Partition";

    int pivot = node->data.back();
    int i = -1; 

    // --- 1. Partition Step ---
    for (int j = 0; j < node->data.size() - 1; j++) {
        UIState state = pollEventsAndCheckUI(window, font, animationDelay, viewOffsetX, minTreeX, maxTreeX, viewOffsetY, maxTreeY, isPanning, lastPanMousePos);
        if (state.shouldClose || state.resetPressed || state.newArrayPressed) return false;
        map<int, Color> highlights;
        highlights[j] = Color::Yellow; 
        highlights[node->data.size() - 1] = Color::Magenta; 
        if (i >= 0) highlights[i] = Color::Blue; 
        drawTreeState(window, font, root, node, animationDelay, viewOffsetX, viewOffsetY, highlights);
        sleep(milliseconds(animationDelay));

        if (node->data[j] < pivot) {
            i++;
            swap(node->data[i], node->data[j]);
            highlights[i] = Color::Red;
            highlights[j] = Color::Red;
            drawTreeState(window, font, root, node, animationDelay, viewOffsetX, viewOffsetY, highlights);
            sleep(milliseconds(animationDelay));
        }
    }
    
    swap(node->data[i + 1], node->data[node->data.size() - 1]);
    int pi = i + 1; 
    int pivotValue = node->data[pi];
    
    map<int, Color> highlights;
    highlights[pi] = Color::Red; 
    drawTreeState(window, font, root, node, animationDelay, viewOffsetX, viewOffsetY, highlights);
    sleep(milliseconds(animationDelay));
    node->isActive = false;
    
    // --- 2. Divide Step (Create Children) ---
    if (pi > 0) {
        node->left = new Node();
        node->left->data = vector<int>(node->data.begin(), node->data.begin() + pi);
    }
    if (pi + 1 < node->data.size()) {
        node->right = new Node();
        node->right->data = vector<int>(node->data.begin() + pi + 1, node->data.end());
    }

    // Recalculate layout
    minTreeX = FLT_MAX; maxTreeX = -FLT_MAX; maxTreeY = 0;
    calculateTreeLayout(root, WINDOW_WIDTH / 2, 100, WINDOW_WIDTH / 4, 120, minTreeX, maxTreeX, maxTreeY);
    drawTreeState(window, font, root, nullptr, animationDelay, viewOffsetX, viewOffsetY, {}); 
    sleep(milliseconds(500)); 

    // --- 3. Recurse Step ---
    if (!animateQuickSort(window, font, node->left, animationDelay, root, viewOffsetX, minTreeX, maxTreeX, viewOffsetY, maxTreeY, isPanning, lastPanMousePos)) return false;
    if (!animateQuickSort(window, font, node->right, animationDelay, root, viewOffsetX, minTreeX, maxTreeX, viewOffsetY, maxTreeY, isPanning, lastPanMousePos)) return false;

    // --- 4. Combine Step (The animated "merge") ---
    node->isSorted = true;
    node->title = "Combining";
    node->isActive = true;

    // 1. Build the final sorted data from children
    vector<int> sortedData;
    if (node->left) {
        sortedData.insert(sortedData.end(), node->left->data.begin(), node->left->data.end());
    }
    sortedData.push_back(pivotValue); // Add the pivot back
    if (node->right) {
        sortedData.insert(sortedData.end(), node->right->data.begin(), node->right->data.end());
    }

    // 2. Animate the parent node filling up one-by-one
    for (size_t k = 0; k < sortedData.size(); ++k) {
        // Check for exit
        UIState state = pollEventsAndCheckUI(window, font, animationDelay, viewOffsetX, minTreeX, maxTreeX, viewOffsetY, maxTreeY, isPanning, lastPanMousePos);
        if (state.shouldClose || state.resetPressed || state.newArrayPressed) return false;

        // Update the node's data one element at a time
        node->data[k] = sortedData[k];

        // Highlight the newly added element
        map<int, Color> combineHighlights;
        for(size_t j = 0; j <= k; ++j) {
            combineHighlights[j] = Color::Green; // Already sorted
        }
        combineHighlights[k] = Color::Red; // Newly added
        
        drawTreeState(window, font, root, node, animationDelay, viewOffsetX, viewOffsetY, combineHighlights);
        sleep(milliseconds(animationDelay));
    }

    // Final draw
    node->title = "Combined";
    node->isActive = false;
    drawTreeState(window, font, root, node, animationDelay, viewOffsetX, viewOffsetY, {});
    sleep(milliseconds(animationDelay * 2 + 50)); 
    
    return true;
}


// ################################################################
// --- MAIN FUNCTION ---
// ################################################################

int main() {
    RenderWindow window(VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Sorting Visualizer");
    window.setFramerateLimit(60);
    Font font;

    if (!font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
        if (!font.openFromFile("C:/Windows/Fonts/Arial.ttf")) {
            if (!font.openFromFile("/usr/share/fonts/truetype/msttcorefonts/Arial.ttf")) {
                cerr << "Failed to load Arial font!" << endl;
                return 1;
            }
        }
    }

    int animationDelay = 50; 
    string userInput;
    vector<int> arr;
    vector<int> originalArr;
    bool enteringInput = true;
    bool sorted = false;

    enum class VizMode { BARS, TREE };
    VizMode currentMode = VizMode::BARS;
    Node* sortTreeRoot = nullptr;
    
    // ✅ NEW: View state variables for panning and scrolling
    float viewOffsetX = 0.0f;
    float viewOffsetY = 0.0f;
    float maxTreeY = 0.0f;
    float minTreeX = FLT_MAX;
    float maxTreeX = -FLT_MAX;
    bool isPanning = false;
    Vector2i lastPanMousePos;
    
    bool bubbleSortPressed = false, insertionSortPressed = false, selectionSortPressed = false;
    bool quickSortPressed = false, mergeSortPressed = false, resetPressed = false;
    bool newArrayPressed = false; 

    while (window.isOpen()) {
        
        bubbleSortPressed = false; insertionSortPressed = false; selectionSortPressed = false;
        quickSortPressed = false; mergeSortPressed = false; resetPressed = false;
        newArrayPressed = false;
        bool speedUpPressed = false, speedDownPressed = false;

        // --- 1. Event Polling (for main menu) ---
        while (auto event = window.pollEvent()) {
            if (event->is<Event::Closed>()) {
                window.close();
            }

            // Handle scrolling in the main menu
            if (currentMode == VizMode::TREE) {
                if (auto scrollEvent = event->getIf<Event::MouseWheelScrolled>()) {
                    if (scrollEvent->wheel == Mouse::Wheel::Vertical) {
                        float delta = scrollEvent->delta; 
                        viewOffsetY = std::clamp(viewOffsetY - delta * 20.0f, 0.0f, std::max(0.0f, maxTreeY - WINDOW_HEIGHT + 100.0f));
                    }
                }
                // ✅ NEW: Handle Panning in the main menu
                if (auto mbp = event->getIf<Event::MouseButtonPressed>()) {
                    if (mbp->button == Mouse::Button::Left) {
                        isPanning = true;
                        lastPanMousePos = Mouse::getPosition(window);
                    }
                }
                if (auto mbr = event->getIf<Event::MouseButtonReleased>()) {
                    if (mbr->button == Mouse::Button::Left) {
                        isPanning = false;
                    }
                }
                if (event->is<Event::MouseMoved>()) {
                    if (isPanning) {
                        Vector2i currentMousePos = Mouse::getPosition(window);
                        Vector2i delta = currentMousePos - lastPanMousePos;
                        viewOffsetX -= delta.x;
                        lastPanMousePos = currentMousePos;
                        // Clamp
                        float minPan = minTreeX - 50;
                        float maxPan = maxTreeX - WINDOW_WIDTH + 50;
                        viewOffsetX = std::clamp(viewOffsetX, std::min(minPan, maxPan), std::max(minPan, maxPan));
                    }
                }
            }

            if (enteringInput) {
                if (auto textEntered = event->getIf<Event::TextEntered>()) {
                    auto unicode = textEntered->unicode;
                    if (unicode == 8 && !userInput.empty()) userInput.pop_back(); 
                    else if (unicode == 13 || unicode == 10) { 
                        stringstream ss(userInput);
                        int num;
                        arr.clear();
                        while (ss >> num) arr.push_back(num);
                        originalArr = arr;
                        enteringInput = false;
                        sorted = false;
                        currentMode = VizMode::BARS;
                        delete sortTreeRoot; sortTreeRoot = nullptr;
                        viewOffsetX = 0; viewOffsetY = 0; maxTreeY = 0; minTreeX = FLT_MAX; maxTreeX = -FLT_MAX;
                    } 
                    else if (isdigit((char)unicode) || unicode == ' ') {
                        userInput += (char)unicode;
                    }
                }
            }
        }

        if (!enteringInput) {
            Vector2i mouse = Mouse::getPosition(window);
            bool mousePressed = Mouse::isButtonPressed(Mouse::Button::Left);

            RectangleShape r_b({120, 40}); r_b.setPosition({10, 10});
            RectangleShape r_i({120, 40}); r_i.setPosition({140, 10});
            RectangleShape r_s({120, 40}); r_s.setPosition({270, 10});
            RectangleShape r_q({120, 40}); r_q.setPosition({400, 10});
            RectangleShape r_m({120, 40}); r_m.setPosition({530, 10});
            RectangleShape r_reset({120, 40}); r_reset.setPosition({850, 10});
            RectangleShape r_new({120, 40}); r_new.setPosition({980, 10});
            RectangleShape r_speedDown({80, 30}); r_speedDown.setPosition({850, 60});
            RectangleShape r_speedUp({80, 30}); r_speedUp.setPosition({940, 60});

            if (!sorted && mousePressed && r_b.getGlobalBounds().contains(Vector2f(mouse))) bubbleSortPressed = true;
            if (!sorted && mousePressed && r_i.getGlobalBounds().contains(Vector2f(mouse))) insertionSortPressed = true;
            if (!sorted && mousePressed && r_s.getGlobalBounds().contains(Vector2f(mouse))) selectionSortPressed = true;
            if (!sorted && mousePressed && r_q.getGlobalBounds().contains(Vector2f(mouse))) quickSortPressed = true;
            if (!sorted && mousePressed && r_m.getGlobalBounds().contains(Vector2f(mouse))) mergeSortPressed = true;
            if (mousePressed && r_reset.getGlobalBounds().contains(Vector2f(mouse))) resetPressed = true;
            if (mousePressed && r_new.getGlobalBounds().contains(Vector2f(mouse))) newArrayPressed = true;
            if (mousePressed && r_speedDown.getGlobalBounds().contains(Vector2f(mouse))) speedDownPressed = true;
            if (mousePressed && r_speedUp.getGlobalBounds().contains(Vector2f(mouse))) speedUpPressed = true;
        }

        window.clear(Color::Black);

        if (enteringInput) {
            Text prompt(font, "Enter numbers (space-separated): " + userInput); 
            prompt.setCharacterSize(20);
            prompt.setFillColor(Color::White);
            prompt.setPosition({30, 250});
            window.draw(prompt);
        } 
        else {
            if (currentMode == VizMode::BARS) {
                drawSortState_bars(window, font, arr, animationDelay, "", {});
            } else if (sortTreeRoot) {
                drawTree(window, font, sortTreeRoot, viewOffsetX, viewOffsetY);
                drawGlobalUI(window, font, animationDelay);
            }
            
            drawButton(window, font, "Bubble", {10, 10});
            drawButton(window, font, "Insertion", {140, 10});
            drawButton(window, font, "Selection", {270, 10});
            drawButton(window, font, "Quick", {400, 10});
            drawButton(window, font, "Merge", {530, 10});
            
            if (speedDownPressed) animationDelay = min(1000, animationDelay - 50);
            if (speedUpPressed) animationDelay = max(0, animationDelay + 50);
            
            auto resetView = [&]() {
                arr = originalArr; 
                sorted = false; 
                currentMode = VizMode::BARS; 
                delete sortTreeRoot; sortTreeRoot = nullptr;
                viewOffsetX = 0; viewOffsetY = 0; maxTreeY = 0; minTreeX = FLT_MAX; maxTreeX = -FLT_MAX;
            };

            if (resetPressed) { resetView(); }
            if (newArrayPressed) {
                enteringInput = true; 
                sorted = false;
                delete sortTreeRoot; sortTreeRoot = nullptr;
                viewOffsetX = 0; viewOffsetY = 0; maxTreeY = 0; minTreeX = FLT_MAX; maxTreeX = -FLT_MAX;
            }

            // --- Start Bar Sorts ---
            if (bubbleSortPressed) { 
                currentMode = VizMode::BARS;
                delete sortTreeRoot; sortTreeRoot = nullptr;
                viewOffsetX = 0; viewOffsetY = 0;
                arr = originalArr;
                sorted = bubbleSort_bars(window, font, arr, animationDelay); 
            }
            if (insertionSortPressed) { 
                currentMode = VizMode::BARS; 
                delete sortTreeRoot; sortTreeRoot = nullptr;
                viewOffsetX = 0; viewOffsetY = 0;
                arr = originalArr;
                sorted = insertionSort_bars(window, font, arr, animationDelay); 
            }
            if (selectionSortPressed) { 
                currentMode = VizMode::BARS; 
                delete sortTreeRoot; sortTreeRoot = nullptr;
                viewOffsetX = 0; viewOffsetY = 0;
                arr = originalArr;
                sorted = selectionSort_bars(window, font, arr, animationDelay); 
            }
            
            // --- Start Tree Sorts ---
            if (mergeSortPressed) {
                currentMode = VizMode::TREE;
                arr = originalArr;
                viewOffsetX = 0; viewOffsetY = 0; maxTreeY = 0; minTreeX = FLT_MAX; maxTreeX = -FLT_MAX;
                delete sortTreeRoot;
                sortTreeRoot = new Node{originalArr, "Root"}; // ✅ NEW: Start with root
                calculateTreeLayout(sortTreeRoot, WINDOW_WIDTH / 2, 100, WINDOW_WIDTH / 4, 120, minTreeX, maxTreeX, maxTreeY);
                // ✅ NEW: Call the new recursive function
                sorted = animateMergeSort(window, font, sortTreeRoot, animationDelay, sortTreeRoot, viewOffsetX, minTreeX, maxTreeX, viewOffsetY, maxTreeY, isPanning, lastPanMousePos);
                if (!sorted) { resetView(); }
            }
            if (quickSortPressed) {
                currentMode = VizMode::TREE;
                arr = originalArr;
                viewOffsetX = 0; viewOffsetY = 0; maxTreeY = 0; minTreeX = FLT_MAX; maxTreeX = -FLT_MAX;
                delete sortTreeRoot;
                sortTreeRoot = new Node{originalArr, "Root"}; 
                calculateTreeLayout(sortTreeRoot, WINDOW_WIDTH / 2, 100, WINDOW_WIDTH / 4, 120, minTreeX, maxTreeX, maxTreeY);
                sorted = animateQuickSort(window, font, sortTreeRoot, animationDelay, sortTreeRoot, viewOffsetX, minTreeX, maxTreeX, viewOffsetY, maxTreeY, isPanning, lastPanMousePos);
                if (!sorted) { resetView(); }
            }
        }

        window.display();
    }
    
    delete sortTreeRoot; 
    return 0;
}