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
  int id;
  int chips;
  int numCardsInHand;
  int hasFolded;
  int hasBet;
  struct Card hand[5];
};

struct Dealer {
  struct Card hand[5];
  int numCardsInHand;
};

struct Game {
  struct Player players[4];
  struct Dealer dealer;
  int numPlayers;
  int currentPlayer;
  int pot;
  int minBet;
};

static const char *faceNames[] = {"Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King", "Ace"};
static const char *suitNames[] = {"Hearts", "Diamonds", "Clubs", "Spades"};
static const char *rankNames[] = {"High Card", "One Pair", "Two Pair", "Three of a Kind", "Straight", "Flush", "Full House", "Four of a Kind", "Straight Flush", "Royal Flush"};

struct Deck deck;

struct Card getCard();
struct Rank getHandRank(struct Card hand[], int numCards);
struct Rank evaluatePlayerHand(const struct Player *player, const struct Dealer *dealer);

int main() {
  srand(time(NULL));

  struct Game game;
  printf("Insert the number of players (2-4): ");
  if (scanf("%d", &game.numPlayers) != 1 || game.numPlayers < 2 || game.numPlayers > 4) {
    printf("Invalid number of players. Please enter a number between 2 and 4.\n");
    return 1;
  }
  game.currentPlayer = 0;
  game.pot = 0;
  game.minBet = 10;
  
  // game is separated in rounds and turns, rounds reset the dealed cards and turns are the actions of the players, each player can bet, fold or check
  // the game continues until only one player is left or all players have bet the same amount
  int round = 1;

  while (1) { // Rounds
    struct Deck newDeck = {{0}, 0};
    deck = newDeck; // reset the deck for each round
    struct Dealer dealer;
    dealer.numCardsInHand = 3;
    for (int i = 0; i < dealer.numCardsInHand; i++) {
      dealer.hand[i] = getCard();
    }

    for (int i = 0; i < game.numPlayers; i++) {
      game.players[i].id = i + 1;
      game.players[i].chips = 20; // starting chips
      for (int j = 0; j < 2; j++)
        game.players[i].hand[j] = getCard();
      game.players[i].numCardsInHand = 2;
      game.players[i].hasFolded = 0;
      game.players[i].hasBet = 0;
    }

    int turn = 1;
    
    while (1) { // Turn
      struct Player *currentPlayer = &game.players[game.currentPlayer];
      // check if one player is left, if so end the round
      int activePlayers = 0;
      for (int i = 0; i < game.numPlayers; i++) {
        if (!game.players[i].hasFolded) {
          activePlayers++;
        }
      }
      if (activePlayers == 1) {
        currentPlayer->chips += game.pot;
        break;
      }
      // check if the current player has folded, if so skip their turn
      if (currentPlayer->hasFolded) {
        game.currentPlayer++; // Move to the next player's turn
        if (game.currentPlayer >= game.numPlayers) {
          game.currentPlayer = 0;
          turn++;
        }
        continue;
      }

      system("clear"); // Clear the console fto prevent cheating by looking at the previous player's hand

      if (turn > 4) { // After the fourth turn, end the round
        // determine the winner and distribute the pot
        int bestRankValue = 0;
        int bestRankFace = 0;
        int bestRankSuit = 0;
        int winnerId = 0;

        for (int i = 0; i < game.numPlayers; i++) {
          if (!game.players[i].hasFolded) {
            struct Rank playerRank = evaluatePlayerHand(&game.players[i], &dealer);
            if (playerRank.rankValue > bestRankValue || 
                (playerRank.rankValue == bestRankValue && playerRank.rankFace > bestRankFace) || 
                (playerRank.rankValue == bestRankValue && playerRank.rankFace == bestRankFace && playerRank.rankSuit > bestRankSuit)) {
              bestRankValue = playerRank.rankValue;
              bestRankFace = playerRank.rankFace;
              bestRankSuit = playerRank.rankSuit;
              winnerId = game.players[i].id;
            }
            printf("> Player %d has a %s\n", game.players[i].id, rankNames[playerRank.rankValue - 1]);
          }
        }
        printf("\n");
        printf("Player %d wins the round with a %s!\n \n", winnerId, rankNames[bestRankValue - 1]);
        for (int i = 0; i < game.numPlayers; i++) {
          if (game.players[i].id == winnerId) {
            game.players[i].chips += game.pot;
          }
        }
        break;
      }

      printf("[Turn %d] Player %d's turn. Chips: %d\n", turn, currentPlayer->id, currentPlayer->chips);
      printf("Dealer's hand:\n");
      for (int i = 0; i < dealer.numCardsInHand; i++) {
        if (dealer.hand[i].suit == 0 || dealer.hand[i].suit == 1) {
          printf(RED);
        } else {
          printf(BLACK);
        }
        printf("  %s of %s\n", faceNames[dealer.hand[i].face], suitNames[dealer.hand[i].suit]);
        printf(RESET);
      }

      printf("Your hand (%s):\n", rankNames[evaluatePlayerHand(currentPlayer, &dealer).rankValue - 1]);
      for (int i = 0; i < currentPlayer->numCardsInHand; i++) {
        if (currentPlayer->hand[i].suit == 0 || currentPlayer->hand[i].suit == 1) {
          printf(RED);
        } else {
          printf(BLACK);
        }
        printf("  %s of %s\n", faceNames[currentPlayer->hand[i].face], suitNames[currentPlayer->hand[i].suit]);
        printf(RESET);
      }
      if (turn == 1) {
        printf("Choose an action: 1) Bet 2) Fold 3) Check\n");
      } else {
        printf("Choose an action: 1) Raise 2) Fold 3) Check\n");
      }
      
      int action;
      if (scanf("%d", &action) != 1 || action < 1 || action > 3) {
        printf("Invalid action. Please enter 1, 2, or 3.\n");
        continue;
      }

      if (action == 2) { // Fold
        currentPlayer->hasFolded = 1;
        system("clear");
        printf("Player %d folds.\n", currentPlayer->id);
      } 

      game.currentPlayer++; // Move to the next player's turn
      
      if (game.currentPlayer >= game.numPlayers) {
        game.currentPlayer = 0;
        turn++;

        if (dealer.numCardsInHand < 5) {
          dealer.hand[dealer.numCardsInHand] = getCard();
          dealer.numCardsInHand++;
        }
        for (int i = 0; i < game.numPlayers; i++) {
          if (game.players[i].numCardsInHand < 5) {
            game.players[i].hand[game.players[i].numCardsInHand] = getCard();
            game.players[i].numCardsInHand++;
          }
        }
      }
    }
    turn = 1; // reset the turn counter for the next round
    printf("Round %d ended. Pot: %d\n", round , game.pot);
    printf("Do you want to play another round? (1 for Yes, 0 for No): ");
    int playAgain;
    if (scanf("%d", &playAgain) != 1 || playAgain < 0 || playAgain > 1) {
      printf("Invalid input. Please enter 1 for Yes or 0 for No.\n");
      return 1;
    }
    if (playAgain == 0) {
      break;
    }
  }

  return 0;
}

struct Card getCard() {
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
    if (handSuitCounts[i] >= 5) {
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

struct Rank evaluatePlayerHand(const struct Player *player, const struct Dealer *dealer) {
  struct Card combinedHand[player->numCardsInHand + dealer->numCardsInHand];
  for (int i = 0; i < player->numCardsInHand; i++) {
    combinedHand[i] = player->hand[i];
  }
  for (int i = 0; i < dealer->numCardsInHand; i++) {
    combinedHand[player->numCardsInHand + i] = dealer->hand[i];
  }
  return getHandRank(combinedHand, player->numCardsInHand + dealer->numCardsInHand);
}