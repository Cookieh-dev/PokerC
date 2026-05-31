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

struct Deck {
  struct Card dealtCards[52];
  int numCardsDealt;
};

static const char *faceNames[] = {"Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King", "Ace"};
static const char *suitNames[] = {"Hearts", "Diamonds", "Clubs", "Spades"};

struct Player {
  char name[50];
  struct Card hand[5];
};

struct Deck deck;


struct Card dealCard();

int main() {
  srand(time(NULL));

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
          deck.numCardsDealt++;
          deck.dealtCards[deck.numCardsDealt] = cardToDeal;
          return cardToDeal;
      }
  }
}
