#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

// All game logic inlined here to avoid OJ git URL prepending issues

struct Node;
struct GameState;
class Card;
class Pile;

enum SkillCardType {
    MULTIPLIER,
    HEAD_BUTT,
};

class Card {
public:
    explicit Card() = default;
    virtual ~Card() = default;
    virtual void play(GameState& game_state) = 0;
};

class ScoreCard: public Card {
private:
    int point_;
public:
    explicit ScoreCard(int point) : point_(point) {}
    void play(GameState &game_state) override;
    ~ScoreCard() override = default;
};

class SkillCard: public Card {
private:
    SkillCardType skill_card_type_;
public:
    explicit SkillCard(SkillCardType skill_card_type) : skill_card_type_(skill_card_type) {}
    void play(GameState &game_state) override;
    ~SkillCard() override = default;
};

class PowerCard: public Card {
private:
    int plus_count_;
public:
    explicit PowerCard(int plus_count) : plus_count_(plus_count) {}
    void play(GameState &game_state) override;
    ~PowerCard() override = default;
};

struct Node {
    Card* card;
    Node* prev;
    Node* next;
    Node(Card* c = nullptr) : card(c), prev(nullptr), next(nullptr) {}
};

class Pile {
private:
    int size_;
    Node* head_;
    Node* tail_;
public:
    friend void outShuffle(GameState&);
    friend void inShuffle(GameState&);
    friend void oddEvenShuffle(GameState&);
    friend class GameController;

    Pile();
    ~Pile();
    int size() const { return size_; }
    bool empty() const { return size_ == 0; }
    void appendCard(Card* card);
    void prependCard(Card* card);
    Card* popFront();
    Card* popBack();
    Card* removeNth(int n);
};

struct GameState {
    Pile hand_{};
    Pile draw_pile_{};
    Pile discard_pile_{};
    int total_score_;
    int power_bonus_;
    int multiplier_;
    GameState() : total_score_(0), power_bonus_(0), multiplier_(1) {}
};

void outShuffle(GameState &game_state);
void inShuffle(GameState &game_state);
void oddEvenShuffle(GameState &game_state);

class GameController {
private:
    GameState game_state_;
    void (*shuffle_)(GameState&);
public:
    explicit GameController(int mode);
    void draw();
    void play(int card_to_play);
    void finish();
    void shuffle();
    int queryScore();
    int queryDrawPileSize() const { return game_state_.draw_pile_.size(); }
    int queryHandSize() const { return game_state_.hand_.size(); }
    int queryDiscardPileSize() const { return game_state_.discard_pile_.size(); }
    Pile& drawPile() { return game_state_.draw_pile_; }
};

// === Shuffle implementations ===
void outShuffle(GameState& gs) {
    Pile& draw = gs.draw_pile_;
    Pile& discard = gs.discard_pile_;
    if (discard.empty()) return;
    std::vector<Card*> cards;
    Node* current = discard.head_->next;
    while (current != discard.tail_) {
        cards.push_back(current->card);
        Node* next = current->next;
        delete current;
        current = next;
    }
    discard.head_->next = discard.tail_;
    discard.tail_->prev = discard.head_;
    discard.size_ = 0;
    for (auto* card : cards) {
        Node* node = new Node(card);
        node->prev = draw.tail_->prev;
        node->next = draw.tail_;
        draw.tail_->prev->next = node;
        draw.tail_->prev = node;
        draw.size_++;
    }
}

void inShuffle(GameState& gs) {
    Pile& draw = gs.draw_pile_;
    Pile& discard = gs.discard_pile_;
    if (discard.empty()) return;
    std::vector<Card*> cards;
    Node* current = discard.head_->next;
    while (current != discard.tail_) {
        cards.push_back(current->card);
        Node* next = current->next;
        delete current;
        current = next;
    }
    discard.head_->next = discard.tail_;
    discard.tail_->prev = discard.head_;
    discard.size_ = 0;
    for (int i = (int)cards.size() - 1; i >= 0; i--) {
        Node* node = new Node(cards[i]);
        node->prev = draw.tail_->prev;
        node->next = draw.tail_;
        draw.tail_->prev->next = node;
        draw.tail_->prev = node;
        draw.size_++;
    }
}

void oddEvenShuffle(GameState& gs) {
    Pile& draw = gs.draw_pile_;
    Pile& discard = gs.discard_pile_;
    if (discard.empty()) return;
    std::vector<std::pair<int, Card*>> cards;
    Node* current = discard.head_->next;
    int idx = 1;
    while (current != discard.tail_) {
        cards.push_back({idx, current->card});
        Node* next = current->next;
        delete current;
        current = next;
        idx++;
    }
    discard.head_->next = discard.tail_;
    discard.tail_->prev = discard.head_;
    discard.size_ = 0;
    for (auto& p : cards) {
        if (p.first % 2 == 1) {
            Node* node = new Node(p.second);
            node->prev = draw.tail_->prev;
            node->next = draw.tail_;
            draw.tail_->prev->next = node;
            draw.tail_->prev = node;
            draw.size_++;
        }
    }
    for (auto& p : cards) {
        if (p.first % 2 == 0) {
            Node* node = new Node(p.second);
            node->prev = draw.tail_->prev;
            node->next = draw.tail_;
            draw.tail_->prev->next = node;
            draw.tail_->prev = node;
            draw.size_++;
        }
    }
}

// === Card implementations ===
void ScoreCard::play(GameState &gs) {
    int score = (point_ + gs.power_bonus_) * gs.multiplier_;
    gs.total_score_ += score;
    gs.multiplier_ = 1;
}

void SkillCard::play(GameState &gs) {
    if (skill_card_type_ == MULTIPLIER) {
        gs.multiplier_ += 1;
    } else if (skill_card_type_ == HEAD_BUTT) {
        if (!gs.discard_pile_.empty()) {
            Card* card = gs.discard_pile_.popBack();
            gs.draw_pile_.prependCard(card);
        }
    }
}

void PowerCard::play(GameState &gs) {
    gs.power_bonus_ += plus_count_;
}

// === Pile implementations ===
Pile::Pile() {
    head_ = new Node();
    tail_ = new Node();
    head_->next = tail_;
    tail_->prev = head_;
    size_ = 0;
}

Pile::~Pile() {
    Node* current = head_->next;
    while (current != tail_) {
        Node* next = current->next;
        delete current->card;
        delete current;
        current = next;
    }
    delete head_;
    delete tail_;
}

void Pile::appendCard(Card* card) {
    Node* node = new Node(card);
    node->prev = tail_->prev;
    node->next = tail_;
    tail_->prev->next = node;
    tail_->prev = node;
    size_++;
}

void Pile::prependCard(Card* card) {
    Node* node = new Node(card);
    node->next = head_->next;
    node->prev = head_;
    if (head_->next != nullptr) {
        head_->next->prev = node;
    } else {
        tail_->prev = node;
    }
    head_->next = node;
    size_++;
}

Card* Pile::popFront() {
    if (size_ == 0) return nullptr;
    Node* node = head_->next;
    Card* card = node->card;
    head_->next = node->next;
    node->next->prev = head_;
    node->next = nullptr;
    node->prev = nullptr;
    size_--;
    delete node;
    return card;
}

Card* Pile::popBack() {
    if (size_ == 0) return nullptr;
    Node* node = tail_->prev;
    Card* card = node->card;
    tail_->prev = node->prev;
    node->prev->next = tail_;
    node->next = nullptr;
    node->prev = nullptr;
    size_--;
    delete node;
    return card;
}

Card* Pile::removeNth(int n) {
    if (n < 1 || n > size_) return nullptr;
    Node* current = head_->next;
    for (int i = 1; i < n; i++) {
        current = current->next;
    }
    Card* card = current->card;
    current->prev->next = current->next;
    current->next->prev = current->prev;
    current->next = nullptr;
    current->prev = nullptr;
    size_--;
    delete current;
    return card;
}

// === GameController implementations ===
GameController::GameController(int mode) {
    if (mode == 1) shuffle_ = outShuffle;
    else if (mode == 2) shuffle_ = inShuffle;
    else if (mode == 3) shuffle_ = oddEvenShuffle;
}

void GameController::draw() {
    for (int i = 0; i < 5; i++) {
        if (game_state_.draw_pile_.empty()) {
            if (!game_state_.discard_pile_.empty()) {
                shuffle_(game_state_);
            } else {
                break;
            }
        }
        Card* card = game_state_.draw_pile_.popFront();
        game_state_.hand_.appendCard(card);
    }
}

void GameController::play(int card_to_play) {
    Card* card = game_state_.hand_.removeNth(card_to_play);
    if (card == nullptr) return;
    card->play(game_state_);
    if (dynamic_cast<ScoreCard*>(card) != nullptr || dynamic_cast<SkillCard*>(card) != nullptr) {
        game_state_.discard_pile_.appendCard(card);
    } else if (dynamic_cast<PowerCard*>(card) != nullptr) {
        delete card;
    }
}

void GameController::shuffle() {
    shuffle_(game_state_);
}

void GameController::finish() {
    while (!game_state_.hand_.empty()) {
        Card* card = game_state_.hand_.popFront();
        game_state_.discard_pile_.appendCard(card);
    }
}

int GameController::queryScore() {
    return game_state_.total_score_;
}

// === Main program ===
void loadCards(Pile& draw_pile) {
    int n;
    std::cin >> n;
    std::cin.ignore();
    for (int i = 0; i < n; ++i) {
        std::string card_type;
        std::cin >> card_type;
        Card *card = nullptr;
        if (card_type == "SCORE") {
            int point;
            std::cin >> point;
            card = new ScoreCard(point);
        } else if (card_type == "SKILL") {
            std::string skill_type;
            std::cin >> skill_type;
            if (skill_type == "Multiplier") {
                card = new SkillCard(SkillCardType::MULTIPLIER);
            } else if (skill_type == "HeadButt") {
                card = new SkillCard(SkillCardType::HEAD_BUTT);
            }
        } else if (card_type == "POWER") {
            int plus_count;
            std::cin >> plus_count;
            card = new PowerCard(plus_count);
        }
        if (card == nullptr) {
            throw std::runtime_error("Invalid Operation");
        }
        draw_pile.appendCard(card);
    }
}

void processCommands(GameController& game) {
    std::string command;
    while (std::cin >> command) {
        if (command == "draw") {
            game.draw();
        } else if (command == "play") {
            int card_index;
            std::cin >> card_index;
            game.play(card_index);
        } else if (command == "finish") {
            game.finish();
        } else if (command == "exit") {
            break;
        } else if (command == "query") {
            std::string query_type;
            std::cin >> query_type;
            if (query_type == "score") {
                std::cout << game.queryScore() << std::endl;
            } else if (query_type == "hand") {
                std::cout << game.queryHandSize() << std::endl;
            } else if (query_type == "draw_pile") {
                std::cout << game.queryDrawPileSize() << std::endl;
            } else if (query_type == "discard_pile") {
                std::cout << game.queryDiscardPileSize() << std::endl;
            } else {
                throw std::runtime_error("Invalid Operation");
            }
        } else {
            throw std::runtime_error("Invalid Operation");
        }
    }
}

int main() {
    int shuffle_mode;
    std::cin >> shuffle_mode;
    auto game_controller = GameController(shuffle_mode);
    loadCards(game_controller.drawPile());
    processCommands(game_controller);
}
