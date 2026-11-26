#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>
#include <algorithm>

struct Node;
struct GameState;
class Card;
class Pile;

/**
 * Skill card type
 * MULTIPLIER: Increases the score multiplier for the next score card
 * HEAD_BUTT: Moves the card at the bottom (tail) of the discard pile to the top (head) of the draw pile
 */
enum SkillCardType {
    MULTIPLIER,
    HEAD_BUTT,
};

/** 
 * Card base class
 */
class Card {
public:
    explicit Card() = default;
    virtual ~Card() = default;
    virtual void play(GameState& game_state) = 0;
};

/** 
 * Score card class
 */
class ScoreCard: public Card {
private:
    int point_;
public:
    explicit ScoreCard(int point);
    void play(GameState &game_state) override;
    ~ScoreCard() override = default;
};

/** 
 * Skill card class
 */
class SkillCard: public Card {
private:
    SkillCardType skill_card_type_;
public:
    explicit SkillCard(SkillCardType skill_card_type);
    void play(GameState &game_state) override;
    ~SkillCard() override = default;
};

/**
 * Power card class
 */
class PowerCard: public Card {
private:
    int plus_count_;
public:
    explicit PowerCard(int plus_count);
    void play(GameState &game_state) override;
    ~PowerCard() override = default;
};

/** 
 * Linked list node structure
 */
struct Node {
    Card* card;
    Node* prev;
    Node* next;

    Node(Card* c = nullptr) : card(c), prev(nullptr), next(nullptr) {}
};

/** 
 * Pile class
 * All three piles are implemented as doubly linked lists with head and tail sentinels
 */
class Pile {
private:
    int size_;
    Node* head_;  // sentinel head (dummy node before first real node)
    Node* tail_;  // sentinel tail (dummy node after last real node)

public:
    friend void outShuffle(GameState&);
    friend void inShuffle(GameState&);
    friend void oddEvenShuffle(GameState&);
    friend class GameController;

    Pile();
    ~Pile();

    int size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    void appendCard(Card* card);
    
    // Insert a card at the front (head) of the pile
    void prependCard(Card* card);
    
    // Remove and return the first real card (after head sentinel)
    Card* popFront();
    
    // Remove and return the last real card (before tail sentinel)
    Card* popBack();
    
    // Remove the nth card (1-indexed from head)
    Card* removeNth(int n);
};

/**
 * Game state structure
 */
struct GameState {
    Pile hand_{};
    Pile draw_pile_{};
    Pile discard_pile_{};
    int total_score_;
    int power_bonus_;
    int multiplier_;

    GameState() : total_score_(0), power_bonus_(0), multiplier_(1) {}
};

/** 
 * Shuffle function declarations
 */
void outShuffle(GameState &game_state);
void inShuffle(GameState &game_state);
void oddEvenShuffle(GameState &game_state);

/**
 * Game controller class
 */
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

    int queryDrawPileSize() const {
        return game_state_.draw_pile_.size();
    }

    int queryHandSize() const {
        return game_state_.hand_.size();
    }

    int queryDiscardPileSize() const {
        return game_state_.discard_pile_.size();
    }

    Pile& drawPile() {
        return game_state_.draw_pile_;
    }
};

// ================= 洗牌函数实现 ===================

/**
 * Shuffle the discard pile into the draw pile in order
 */
void outShuffle(GameState& game_state) {
    Pile& draw = game_state.draw_pile_;
    Pile& discard = game_state.discard_pile_;
    
    if (discard.empty()) return;
    
    // Collect all cards from discard
    std::vector<Card*> cards;
    Node* current = discard.head_->next;
    while (current != discard.tail_) {
        cards.push_back(current->card);
        Node* next = current->next;
        delete current;
        current = next;
    }
    
    // Clear discard pile
    discard.head_->next = discard.tail_;
    discard.tail_->prev = discard.head_;
    discard.size_ = 0;
    
    // Append in order to draw pile
    for (auto* card : cards) {
        Node* node = new Node(card);
        node->prev = draw.tail_->prev;
        node->next = draw.tail_;
        draw.tail_->prev->next = node;
        draw.tail_->prev = node;
        draw.size_++;
    }
}

/**
 * Shuffle the discard pile into the draw pile in reverse order
 */
void inShuffle(GameState& game_state) {
    Pile& draw = game_state.draw_pile_;
    Pile& discard = game_state.discard_pile_;
    
    if (discard.empty()) return;
    
    // Collect all cards from discard
    std::vector<Card*> cards;
    Node* current = discard.head_->next;
    while (current != discard.tail_) {
        cards.push_back(current->card);
        Node* next = current->next;
        delete current;
        current = next;
    }
    
    // Clear discard pile
    discard.head_->next = discard.tail_;
    discard.tail_->prev = discard.head_;
    discard.size_ = 0;
    
    // Append in reverse order to draw pile
    for (int i = (int)cards.size() - 1; i >= 0; i--) {
        Node* node = new Node(cards[i]);
        node->prev = draw.tail_->prev;
        node->next = draw.tail_;
        draw.tail_->prev->next = node;
        draw.tail_->prev = node;
        draw.size_++;
    }
}

/**
 * Separate the discard pile by odd and even positions, with odd positions first and even positions last
 */
void oddEvenShuffle(GameState& game_state) {
    Pile& draw = game_state.draw_pile_;
    Pile& discard = game_state.discard_pile_;
    
    if (discard.empty()) return;
    
    // Collect all cards from discard with their 1-indexed positions
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
    
    // Clear discard pile
    discard.head_->next = discard.tail_;
    discard.tail_->prev = discard.head_;
    discard.size_ = 0;
    
    // Append odd-indexed cards first, then even-indexed cards
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
    
// ======================================================


// ================= Card Class Implementation ===========================

// === Score Card Class Implementation ===
ScoreCard::ScoreCard(int point) : point_(point) {
}

void ScoreCard::play(GameState &game_state) {
    int score = (point_ + game_state.power_bonus_) * game_state.multiplier_;
    game_state.total_score_ += score;
    game_state.multiplier_ = 1;
}
// ===================

// === Skill Card Class Implementation ===
SkillCard::SkillCard(SkillCardType skill_card_type) : skill_card_type_(skill_card_type) {
}

void SkillCard::play(GameState &game_state) {
    if (skill_card_type_ == MULTIPLIER) {
        game_state.multiplier_ += 1;
    } else if (skill_card_type_ == HEAD_BUTT) {
        // Move discard pile tail to draw pile head
        if (!game_state.discard_pile_.empty()) {
            Card* card = game_state.discard_pile_.popBack();
            game_state.draw_pile_.prependCard(card);
        }
    }
}
// ===================

// === Power Card Class Implementation ===
PowerCard::PowerCard(int plus_count) : plus_count_(plus_count) {
}

void PowerCard::play(GameState &game_state) {
    game_state.power_bonus_ += plus_count_;
    // Card is NOT deleted here - the caller (GameController::play) handles destruction
}
// ===================

// ======================================================



// ================= Pile Class Implementation ===========================
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
        delete current->card;  // Delete the card - cards are only moved, never duplicated
        delete current;         // Delete the node
        current = next;
    }
    delete head_;
    delete tail_;
}

/**
 * Add a card to the end of the pile
 */
void Pile::appendCard(Card* card) {
    Node* node = new Node(card);
    node->prev = tail_->prev;
    node->next = tail_;
    tail_->prev->next = node;
    tail_->prev = node;
    size_++;
}

/**
 * Insert a card at the front (head) of the pile
 */
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

// ======================================================



// ================= Game Controller Class Implementation ======================

GameController::GameController(int mode){
    if (mode == 1) {
        shuffle_ = outShuffle;
    } else if (mode == 2) {
        shuffle_ = inShuffle;
    } else if (mode == 3) {
        shuffle_ = oddEvenShuffle;
    }
}

void GameController::draw() {
    for (int i = 0; i < 5; i++) {
        if (game_state_.draw_pile_.empty()) {
            // During draw process, if draw pile is empty and discard pile is not empty, trigger shuffle
            if (!game_state_.discard_pile_.empty()) {
                shuffle_(game_state_);
            } else {
                break;  // Both piles empty, can't draw
            }
        }
        Card* card = game_state_.draw_pile_.popFront();
        game_state_.hand_.appendCard(card);
    }
}

void GameController::play(int card_to_play) {
    Card* card = game_state_.hand_.removeNth(card_to_play);
    if (card == nullptr) return;
    
    // Execute card effect
    card->play(game_state_);
    
    // Handle card based on type
    if (dynamic_cast<ScoreCard*>(card) != nullptr || dynamic_cast<SkillCard*>(card) != nullptr) {
        // Score card and skill card go to discard pile
        game_state_.discard_pile_.appendCard(card);
    } else if (dynamic_cast<PowerCard*>(card) != nullptr) {
        // Power card is destroyed
        delete card;
    }
}

void GameController::shuffle() {
    shuffle_(game_state_);
}

void GameController::finish() {
    // Move all remaining hand cards to discard pile
    while (!game_state_.hand_.empty()) {
        Card* card = game_state_.hand_.popFront();
        game_state_.discard_pile_.appendCard(card);
    }
}

int GameController::queryScore() {
    return game_state_.total_score_;
}

#endif //GAME_H
