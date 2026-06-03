#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RED          "\x1b[31m"
#define BLACK        "\x1b[30m"
#define RESET        "\x1b[0m"

struct Card {
  unsigned char face;
  unsigned char suit;
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
  struct Card hand[5];
  int numCardsInHand;
  int id;
  int chips;
  int hasFolded;
  int hasBet;
};

struct Dealer { // sim eu sei que isso é so mais um jogador mas fica mais bonito assim 
  struct Card hand[5];
  int numCardsInHand;
};

struct Game {
  struct Player players[4];
  struct Dealer dealer;
  int round;
  int turn;
  int numPlayers;
  int activePlayers;
  int currentPlayer;
  int pot;
  int minBet;
};

static const char *faceNames[] = {"Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King", "Ace"};
static const char *suitNames[] = {"Hearts", "Diamonds", "Clubs", "Spades"};
static const char *rankNames[] = {"High Card", "One Pair", "Two Pair", "Three of a Kind", "Straight", "Flush", "Full House", "Four of a Kind", "Straight Flush", "Royal Flush"};

struct Deck deck;
struct Game game;

void initGame();
void initRound();
int progressRound();
int progressTurn();
void determineWinner();
struct Card getCard();
struct Rank getHandRank(struct Card hand[], int numCards);
struct Rank evaluatePlayerHand(const struct Player *player, const struct Dealer *dealer);
void printHand(struct Card hand[], int numCards);
void clear_terminal();

int main() {
  srand(time(NULL));

  initGame();

  while (1) { // Rounds
    initRound();
    while (1) { // Turn
      struct Player *currentPlayer = &game.players[game.currentPlayer];
      
      game.activePlayers = 0;

      for (int i = 0; i < game.numPlayers; i++) {
        if (!game.players[i].hasFolded) {
          game.activePlayers++;
        }
      }

      if (game.activePlayers == 1) {
        for (int i = 0; i < game.numPlayers; i++) {
          if (game.players[i].hasFolded) {
            printf("> Player %d folded. \n", game.players[i].id);
          }
        }
        currentPlayer->chips += game.pot;
        break;
      }
      
      if (currentPlayer->hasFolded) {
        progressTurn();
        continue;
      }

      clear_terminal();

      if (game.turn > 4) {
        determineWinner();
        break;
      }

      printf("[Turn %d] Player %d's action. Chips: %d\n", game.turn, currentPlayer->id, currentPlayer->chips);
      printf("Dealer's hand:\n");
      printHand(game.dealer.hand, game.dealer.numCardsInHand);

      printf("Your hand (%s):\n", rankNames[evaluatePlayerHand(currentPlayer, &game.dealer).rankValue - 1]);
      printHand(currentPlayer->hand, currentPlayer->numCardsInHand);

      if (game.turn == 1) {
        printf("Choose an action: 1) Check 2) Fold 3) Bet\n");
      } else {
        printf("Choose an action: 1) Check 2) Fold 3) Raise\n");
      }
      
      int action;
      if (scanf("%d", &action) != 1 || action < 1 || action > 3) {
        printf("Invalid action. Please enter 1, 2, or 3.\n");
        continue;
      }

      if (action == 2) { // Fold
        currentPlayer->hasFolded = 1;
        clear_terminal();
      } 
      else if (action == 3) { // Bet or Raise
        
      }

      if (progressTurn()) {
        if (game.dealer.numCardsInHand < 5) {
          game.dealer.hand[game.dealer.numCardsInHand] = getCard();
          game.dealer.numCardsInHand++;
        }
        for (int i = 0; i < game.numPlayers; i++) {
          if (game.players[i].numCardsInHand < 5) {
            game.players[i].hand[game.players[i].numCardsInHand] = getCard();
            game.players[i].numCardsInHand++;
          }
        }
      }
    }
    if (!progressRound()) {
      break;
    }
  }

  return 0;
}

// GAME LOGIC FUNCTIONS

void initGame() {
  printf("Insert the number of players (2-4): ");
  if (scanf("%d", &game.numPlayers) != 1 || game.numPlayers < 2 || game.numPlayers > 4) {
    printf("Invalid number of players. Please enter a number between 2 and 4.\n");
    exit(1);
  }
  game.currentPlayer = 0;
  game.pot = 0;
  game.minBet = 10;
  game.round = 1;
  game.turn = 1;
}

void initRound() {
  struct Deck newDeck = {{0}, 0};
  struct Dealer dealer = {{0}, 0};
  deck = newDeck; // reset the deck for each round
  game.dealer = dealer;

  game.dealer.numCardsInHand = 3;
  for (int i = 0; i < game.dealer.numCardsInHand; i++) {
    game.dealer.hand[i] = getCard();
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

  game.turn = 1;
  game.currentPlayer = 0;
}

int progressRound() {
  printf("Round %d ended. Pot: %d\n", game.round , game.pot);
  game.round++;
  printf("Do you want to play another round? (1 for Yes, 0 for No): ");
  int playAgain;
  if (scanf("%d", &playAgain) != 1 || playAgain < 0 || playAgain > 1) {
    printf("Invalid input. Please enter 1 for Yes or 0 for No.\n");
    return 1;
  }
  
  return playAgain;
}

int progressTurn() {
  game.currentPlayer++; // Move to the next player's turn
  if (game.currentPlayer >= game.numPlayers) {
    game.currentPlayer = 0;
    game.turn++;

    return 1;
  }

  return 0;
}

void determineWinner() {
  int bestRankValue = 0;
  int bestRankFace = 0;
  int bestRankSuit = 0;
  int winnerId = 0;

  for (int i = 0; i < game.numPlayers; i++) {
    if (!game.players[i].hasFolded) {
      struct Rank playerRank = evaluatePlayerHand(&game.players[i], &game.dealer);
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
    else {
      printf("> Player %d folded.\n", game.players[i].id);
    }
  }
  printf("\n");
  printf("Player %d wins the round with a %s!\n \n", winnerId, rankNames[bestRankValue - 1]);
  for (int i = 0; i < game.numPlayers; i++) {
    if (game.players[i].id == winnerId) {
      game.players[i].chips += game.pot;
    }
  }
}

// DECK RELATED FUNCTIONS

struct Card getCard() { // >>> NEEDS OPTIMIZATION
  int i;
  int isUnique;
  struct Card cardToDeal;
  
  while(1){
      cardToDeal.face = rand()%13;
      cardToDeal.suit = rand()%4;
      isUnique = 1;
      
      for(i = 0; i < deck.numCardsDealt; i++){ // cant use continue because its inside of a for loop
          if(deck.dealtCards[i].face == cardToDeal.face && deck.dealtCards[i].suit == cardToDeal.suit) {
              isUnique = 0;
          }
      }
      
      if(isUnique){
          deck.dealtCards[deck.numCardsDealt] = cardToDeal;
          deck.numCardsDealt++;
          return cardToDeal;
      }
  }
}

struct Rank getHandRank(struct Card hand[], int numCards) {
  int handFaceCounts[13] = {0};
  int handSuitCounts[4] = {0};
  int suitFaceCounts[4][13] = {{0}};

  for (int i = 0; i < numCards; i++) {
    int face = (int)hand[i].face;
    int suit = (int)hand[i].suit;
    handFaceCounts[face]++;
    handSuitCounts[suit]++;
    suitFaceCounts[suit][face]++;
  }

  int isFlush = 0;
  int flushSuit = -1;
  int flushHighFace = -1;
  for (int i = 0; i < 4; i++) {
    if (handSuitCounts[i] < 5) {
      continue;
    }

    int candidateHighFace = -1;
    for (int j = 12; j >= 0; j--) {
      if (suitFaceCounts[i][j] > 0) {
        candidateHighFace = j;
        break;
      }
    }

    if (candidateHighFace < 0) {
      continue;
    }

    if (!isFlush || candidateHighFace > flushHighFace || (candidateHighFace == flushHighFace && i > flushSuit)) {
      isFlush = 1;
      flushSuit = i;
      flushHighFace = candidateHighFace;
    }
  }

  int isStraight = 0;
  int straightHighFace = -1;
  for (int i = 0; i <= 8; i++) {
    if (handFaceCounts[i] > 0 && handFaceCounts[i + 1] > 0 && handFaceCounts[i + 2] > 0 && handFaceCounts[i + 3] > 0 && handFaceCounts[i + 4] > 0) {
      isStraight = 1;
      straightHighFace = i + 4;
      break;
    }
  }
  if (!isStraight && handFaceCounts[12] > 0 && handFaceCounts[0] > 0 && handFaceCounts[1] > 0 && handFaceCounts[2] > 0 && handFaceCounts[3] > 0) {
    isStraight = 1;
    straightHighFace = 3; // A-2-3-4-5 wheel straight
  }

  int isStraightFlush = 0;
  int straightFlushSuit = -1;
  int straightFlushHighFace = -1;
  for (int i = 0; i < 4; i++) {
    if (handSuitCounts[i] < 5) {
      continue;
    }

    for (int j = 0; j <= 8; j++) {
      if (suitFaceCounts[i][j] > 0 && suitFaceCounts[i][j + 1] > 0 && suitFaceCounts[i][j + 2] > 0 && suitFaceCounts[i][j + 3] > 0 && suitFaceCounts[i][j + 4] > 0) {
        int candidateHighFace = j + 4;
        if (!isStraightFlush || candidateHighFace > straightFlushHighFace || (candidateHighFace == straightFlushHighFace && i > straightFlushSuit)) {
          isStraightFlush = 1;
          straightFlushSuit = i;
          straightFlushHighFace = candidateHighFace;
        }
        break;
      }
    }

    if (suitFaceCounts[i][12] > 0 && suitFaceCounts[i][0] > 0 && suitFaceCounts[i][1] > 0 && suitFaceCounts[i][2] > 0 && suitFaceCounts[i][3] > 0) {
      int candidateHighFace = 3;
      if (!isStraightFlush || candidateHighFace > straightFlushHighFace || (candidateHighFace == straightFlushHighFace && i > straightFlushSuit)) {
        isStraightFlush = 1;
        straightFlushSuit = i;
        straightFlushHighFace = candidateHighFace;
      }
    }
  }

  int fourOfAKindFace = -1;
  int threeFaces[2] = {-1, -1};
  int pairFaces[4] = {-1, -1, -1, -1};
  int threeCount = 0;
  int pairCount = 0;

  for (int i = 12; i >= 0; i--) {
    if (handFaceCounts[i] == 4 && fourOfAKindFace < 0) {
      fourOfAKindFace = i;
    } else if (handFaceCounts[i] == 3 && threeCount < 2) {
      threeFaces[threeCount++] = i;
    } else if (handFaceCounts[i] == 2 && pairCount < 4) {
      pairFaces[pairCount++] = i;
    }
  }

  int isFullHouse = 0;
  int fullHouseTripFace = -1;
  int fullHousePairFace = -1;
  if (threeCount >= 2) {
    isFullHouse = 1;
    fullHouseTripFace = threeFaces[0];
    fullHousePairFace = threeFaces[1];
  } else if (threeCount == 1 && pairCount >= 1) {
    isFullHouse = 1;
    fullHouseTripFace = threeFaces[0];
    fullHousePairFace = pairFaces[0];
  }

  struct Rank rank = {0, 0, 0};
  if (isStraightFlush) {
    if (straightFlushHighFace == 12) {
      rank.rankValue = 10; // Royal Flush
    } else {
      rank.rankValue = 9; // Straight Flush
    }
    rank.rankFace = straightFlushHighFace;
    rank.rankSuit = straightFlushSuit;
  } else if (fourOfAKindFace >= 0) {
    rank.rankValue = 8; // Four of a Kind
    rank.rankFace = fourOfAKindFace;
    rank.rankSuit = 0;
    for (int i = 0; i < numCards; i++) {
      if (hand[i].face == fourOfAKindFace && hand[i].suit > rank.rankSuit) {
        rank.rankSuit = hand[i].suit;
      }
    }
  } else if (isFullHouse) {
    rank.rankValue = 7; // Full House
    rank.rankFace = fullHouseTripFace;
    rank.rankSuit = fullHousePairFace;
  } else if (isFlush) {
    rank.rankValue = 6; // Flush
    rank.rankFace = flushHighFace;
    rank.rankSuit = flushSuit;
  } else if (isStraight) {
    rank.rankValue = 5; // Straight
    rank.rankFace = straightHighFace;
    rank.rankSuit = 0;
  } else if (threeCount >= 1) {
    rank.rankValue = 4; // Three of a Kind
    rank.rankFace = threeFaces[0];
    rank.rankSuit = 0;
    for (int i = 12; i >= 0; i--) {
      if (handFaceCounts[i] > 0 && i != threeFaces[0]) {
        rank.rankSuit = i;
        break;
      }
    }
  } else if (pairCount >= 2) {
    rank.rankValue = 3; // Two Pair
    rank.rankFace = pairFaces[0];
    rank.rankSuit = pairFaces[1];
  } else if (pairCount == 1) {
    rank.rankValue = 2; // One Pair
    rank.rankFace = pairFaces[0];
    rank.rankSuit = 0;
    for (int i = 12; i >= 0; i--) {
      if (handFaceCounts[i] > 0 && i != pairFaces[0]) {
        rank.rankSuit = i;
        break;
      }
    }
  } else {
    rank.rankValue = 1; // High Card
    for (int i = 12; i >= 0; i--) {
      if (handFaceCounts[i] > 0) {
        rank.rankFace = i;
        break;
      }
    }
    rank.rankSuit = 0;
    for (int i = 0; i < numCards; i++) {
      if (hand[i].face == rank.rankFace && hand[i].suit > rank.rankSuit) {
        rank.rankSuit = hand[i].suit;
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

// UTIL

void printHand(struct Card hand[], int numCards) {
  for (int i = 0; i < numCards; i++) {
    if (hand[i].suit == 0 || hand[i].suit == 1) {
      printf(RED);
    } else {
      printf(BLACK);
    }
    printf("  %s of %s\n", faceNames[hand[i].face], suitNames[hand[i].suit]);
    printf(RESET);
  }
}

void clear_terminal() {
  const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
  printf("%s", CLEAR_SCREEN_ANSI);
}