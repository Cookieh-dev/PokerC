#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RED          "\x1b[31m"
#define BLACK        "\x1b[30m"
#define RESET        "\x1b[0m"

struct Card {
  char face;
  char suit;
};

struct Rank {
  int rankValue;
  int rankFace;
  int rankSuit;
};

struct Deck {
  struct Card dealtCards[52];
  int numCardsDealt;
};

struct Player {
  char name[50];
  struct Card hand[5];
};

static const char *faceNames[] = {"Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King", "Ace"};
static const char *suitNames[] = {"Hearts", "Diamonds", "Clubs", "Spades"};
static const char *rankNames[] = {"High Card", "One Pair", "Two Pair", "Three of a Kind", "Straight", "Flush", "Full House", "Four of a Kind", "Straight Flush", "Royal Flush"};

struct Deck deck;

struct Card dealCard();
struct Rank getHandRank(struct Card hand[], int numCards);

int main() {
  srand(time(NULL));

  // generate a hand of 5 cards then check its rank
  struct Card testHand[5];
  for (int i = 0; i < 5; i++) {
    testHand[i] = dealCard();
  }
  struct Rank handRank = getHandRank(testHand, 5);
  printf("Hand Rank: %s\n", rankNames[handRank.rankValue - 1]);
  printf(BLACK "Cards in Hand:\n");
  for (int i = 0; i < 5; i++) {
    // highlight repeated cards in red
    int isRepeated = 0;
    for (int j = 0; j < 5; j++) {
      if (i != j && testHand[i].face == testHand[j].face && testHand[i].suit == testHand[j].suit) {
        isRepeated = 1;
        break;
      }
    }
    if (isRepeated) {
      printf(RED);
    }
    else {
      printf(BLACK);
    }
    
    printf("%s of %s\n", faceNames[testHand[i].face], suitNames[testHand[i].suit]);
  }
  return 0;
}

struct Card dealCard() {
  int i;
  int isUnique;
  struct Card cardToDeal;
  while(1){
      cardToDeal.face = rand()%13;
      cardToDeal.suit = rand()%4;
      isUnique = 1;
      
      for(i = 0; i < deck.numCardsDealt; i++){
          if(deck.dealtCards[i].face == cardToDeal.face && deck.dealtCards[i].suit == cardToDeal.suit)
              isUnique = 0;
      }
      
      if(isUnique){
          deck.dealtCards[deck.numCardsDealt] = cardToDeal;
          deck.numCardsDealt++;
          return cardToDeal;
      }
  }
}

struct Rank getHandRank(struct Card hand[], int numCards) {
  // This function will determine the rank of the given hand.
  int handFaceCounts[13] = {0};
  int handSuitCounts[4] = {0};

  for (int i = 0; i < numCards; i++) {
    handFaceCounts[hand[i].face]++;
    handSuitCounts[hand[i].suit]++;
  }

  // check for flush
  int isFlush = 0;
  int flushSuit = 0;

  for (int i = 0; i < 4; i++) {
    if (handSuitCounts[i] == 5) {
      isFlush = 1; // indicates that it's a flush
      flushSuit = i; // store the suit of the flush
      break;
    }
  }

  // check for straight
  int isStraight = 0;
  int straightFace = 0;

  for (int i = 0; i <= 8; i++) {
    if (handFaceCounts[i] > 0 && handFaceCounts[i + 1] > 0 && handFaceCounts[i + 2] > 0 && handFaceCounts[i + 3] > 0 && handFaceCounts[i + 4] > 0) {
      isStraight = 1; // indicates that it's a straight
      straightFace = i; // store the lowest face of the straight
      break;
    }
  }

  // check for other hand ranks (4 of a kind, 3 of a kind, pairs) and its face and suit
  int isFourOfAKind = 0;
  int fourOfAKindFace = 0;
  int fourOfAKindSuit = 0;
  int isThreeOfAKind = 0;
  int threeOfAKindFace = 0;
  int threeOfAKindSuit = 0;
  int pairsCount = 0;
  int pairFaces = 0;
  int pairSuits = 0;

  for (int i = 0; i < 13; i++) {
    if (handFaceCounts[i] == 4) {
      isFourOfAKind = 1;
      fourOfAKindFace = i;
      for (int j = 0; j < 4; j++) {
        if (hand[j].face == i) {
          fourOfAKindSuit += hand[j].suit; // sum of the suits of the four of a kind (for tie-breaking)
        }
      }
    } else if (handFaceCounts[i] == 3) {
      isThreeOfAKind = 1;
      threeOfAKindFace = i;
      for (int j = 0; j < 4; j++) {
        if (hand[j].face == i) {
          threeOfAKindSuit += hand[j].suit; // sum of the suits of the three of a kind (for tie-breaking)
        }
      }
    } else if (handFaceCounts[i] == 2) {
      pairFaces = i;
      for (int j = 0; j < 4; j++) {
        if (hand[j].face == i) {
          pairSuits += hand[j].suit; // sum of the suits of the pairs (for tie-breaking)
        }
      }
      pairsCount++;
    }
  }

  // return the rank of the hand
  struct Rank rank = {0, 0, 0};
  if (isStraight && isFlush && straightFace == 8) { // check for Royal Flush (straight flush with Ace as the highest card)
    rank.rankValue = 10; // Royal Flush
    rank.rankFace = straightFace;
    rank.rankSuit = flushSuit;
  } else if (isStraight && isFlush) {
    rank.rankValue = 9; // Straight Flush
    rank.rankFace = straightFace;
    rank.rankSuit = flushSuit;
  } else if (isFourOfAKind) {
    rank.rankValue = 8; // Four of a Kind
    rank.rankFace = fourOfAKindFace;
    rank.rankSuit = fourOfAKindSuit;
  } else if (isThreeOfAKind && pairsCount == 1) {
    rank.rankValue = 7; // Full House
    rank.rankFace = threeOfAKindFace;
    rank.rankSuit = threeOfAKindSuit + pairSuits; // sum of the suits of the three of a kind and the pair (for tie-breaking)
  } else if (isFlush) {
    rank.rankValue = 6; // Flush
    rank.rankFace = flushSuit; // use the suit of the flush for tie-breaking
  } else if (isStraight) {
    rank.rankValue = 5; // Straight
    rank.rankFace = straightFace; // use the lowest face of the straight for tie-breaking
  } else if (isThreeOfAKind) {
    rank.rankValue = 4; // Three of a Kind
    rank.rankFace = threeOfAKindFace;
    rank.rankSuit = threeOfAKindSuit;
  } else if (pairsCount == 2) {
    rank.rankValue = 3; // Two Pair
    rank.rankFace = pairFaces; // use the face of the higher pair for tie-breaking
    rank.rankSuit = pairSuits; // use the sum of the suits of the pairs for tie-breaking
  } else if (pairsCount == 1) {
    rank.rankValue = 2; // One Pair
    rank.rankFace = pairFaces;
    rank.rankSuit = pairSuits;
  } else {
    rank.rankValue = 1; // High Card
    for (int i = 12; i >= 0; i--) { // check from highest face to lowest for tie-breaking
      if (handFaceCounts[i] > 0) {
        rank.rankFace = i;
        break;
      }
    }
    for (int i = 3; i >= 0; i--) { // check from highest suit to lowest for tie-breaking
      if (handSuitCounts[i] > 0) {
        rank.rankSuit = i;
        break;
      }
    }
  }

  return rank;
}
