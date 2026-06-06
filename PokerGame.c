#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ASSIGNING VARIABLES

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
  int hasLost;
  // add a hasLost to prevent players with 0 or less chips from playing unless they have a lastChance
  // add a lastChance that allows players to go all in when a bet is set higher than their pockets can pay
  int toSpend;
};

struct Dealer { // sim eu sei que isso é so mais um jogador mas fica mais bonito assim 
  struct Card hand[5];
  int numCardsInHand;
};

struct Game {
  struct Player players[5];
  struct Dealer dealer;
  int round;
  int turn;
  int numPlayers;
  int activePlayers;
  int allInPlayers;
  int currentPlayer;
  int currentWinner;
  int firstToBet;
  int pot;
  int minBet;
  int highestBet;
};

// INITING VARIABLES

static const int MAX_PLAYERS = 5; // Do not go above 5 players
static const int MIN_PLAYERS = 2; // Do not go below 2 players
static const int STARTING_CHIPS = 20;

static const char *faceNames[] = {"Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King", "Ace"};
static const char *suitNames[] = {"Clubs", "Diamonds", "Hearts", "Spades"};
static const char *rankNames[] = {"High Card", "One Pair", "Two Pair", "Three of a Kind", "Straight", "Flush", "Full House", "Four of a Kind", "Straight Flush", "Royal Flush"};

struct Deck deck;
struct Game game;

// INITING FUNCTIONS

void initGame();
void initRound();
int progressRound();
void progressTurn();
int determineWinner();

void initDeck();
void shuffleDeck();
struct Card getCard();
struct Rank getHandRank(struct Card hand[], int numCards);
struct Rank evaluatePlayerHand(const struct Player *player, const struct Dealer *dealer);

int playerConfirmation();
int playerAction();
void printHand(struct Card hand[], int numCards);
void printStatus();
void clear_terminal();

// MAIN

int main() {
  srand(time(NULL));

  initGame();

  while (1) { // Rounds

    initRound();

    while (1) { // Turn
      clear_terminal();
      struct Player *currentPlayer = &game.players[game.currentPlayer]; // utility variable

      // TURN START CHECKS

      if (game.activePlayers == 1) {
        for (int i = 0; i < game.numPlayers; i++) {
          if (game.players[i].hasFolded) {
            printf("> Player %d folded. \n", game.players[i].id);
          } else {
            game.currentWinner = game.players[i].id;
          }
        }
        printf("\nPlayer %d wins the round\n\n", game.currentWinner);
        break;
      }

      if (game.allInPlayers == game.activePlayers) {
        for (int i = 4 - game.turn; i > 0; i--) {
          if (game.dealer.numCardsInHand < 5) { // deal dealer card
            game.dealer.hand[game.dealer.numCardsInHand] = getCard();
            game.dealer.numCardsInHand++;
          }
          for (int i = 0; i < game.numPlayers; i++) {
            if (game.players[i].numCardsInHand < 5) { // deal player cards
              game.players[i].hand[game.players[i].numCardsInHand] = getCard();
              game.players[i].numCardsInHand++;
            }
          }
        }
        game.turn = 5; //lazy man :/
      }
      
      if (game.turn > 4) { // Last turn check
        game.currentWinner = determineWinner();

        for (int i = 0; i < game.numPlayers; i++) {
          if (!game.players[i].hasFolded) {
            printf("> Player %d has a %s. \n", game.players[i].id, rankNames[evaluatePlayerHand(&game.players[i], &game.dealer).rankValue - 1]);
          } else {
            printf("> Player %d folded. \n", game.players[i].id);
          }
        }

        printf("\nPlayer %d wins the round with a %s! +%d awarded from the pot\n \n", game.currentWinner, rankNames[evaluatePlayerHand(&game.players[game.currentWinner - 1], &game.dealer).rankValue - 1], game.pot);
        break;
      }

      if (currentPlayer->hasLost) {
        progressTurn();
        continue;
      }

      if (currentPlayer->hasFolded) { // Skip folded players
        progressTurn();
        continue;
      }

      if (currentPlayer->hasBet) { // Skip players who have already bet or raised
        progressTurn();
        currentPlayer->hasBet = 0;
        continue;
      }

      // PLAYER ACTION + HUD
      // playerConfirmation();
      int action = playerAction();
      if (action == 1) { // Check or call
        if (game.highestBet > 0) {
          currentPlayer->toSpend = game.highestBet;
        } else {
          currentPlayer->toSpend = game.minBet;
        }
      } 
      else if (action == 2) { // Fold
        currentPlayer->hasFolded = 1;
      } 
      else if (action == 3) { // Bet or Raise
        for (int i = 0; i < game.numPlayers; i++) {
          if (!game.players[i].hasFolded) {
            game.players[i].hasBet = 0;
          }
        }
        currentPlayer->hasBet = 1;
        game.currentPlayer = 0;
        
        int betAmount;
        while (1) {
          if (game.highestBet > 0) {
            printf("Enter your raise amount (%d-%d): ", game.highestBet + 1, currentPlayer->chips);
            if (scanf("%d", &betAmount) != 1 || betAmount < game.highestBet + 1 || betAmount > currentPlayer->chips) {
              printf("Invalid bet amount. Please enter a number between %d and %d.\n", game.highestBet + 1, currentPlayer->chips);
              continue;
            }
          } else {
            printf("Enter your bet amount (%d-%d): ", game.minBet + 1, currentPlayer->chips);
            if (scanf("%d", &betAmount) != 1 || betAmount < game.minBet + 1 || betAmount > currentPlayer->chips) {
              printf("Invalid bet amount. Please enter a number between %d and %d.\n", game.minBet + 1, currentPlayer->chips);
              continue;
            }
          }
          break;
        }
        
        currentPlayer->toSpend = betAmount;
        game.firstToBet = currentPlayer->id;

        if (betAmount > game.highestBet) {
          game.highestBet = betAmount;
        }

        continue;
      }
      progressTurn();
    }
    if (!progressRound()) {
      break;
    }
  }

  return 0;
}

// GAME LOGIC FUNCTIONS

void initGame() {
  printf("Insert the number of players (%d-%d): ", MIN_PLAYERS, MAX_PLAYERS);
  if (scanf("%d", &game.numPlayers) != 1 || game.numPlayers < MIN_PLAYERS || game.numPlayers > MAX_PLAYERS) {
    printf("Invalid number of players. Please enter a number between %d and %d.\n", MIN_PLAYERS, MAX_PLAYERS);
    exit(1);
  }
  printf("Insert the minimum bet (Maximum %d): ", STARTING_CHIPS);
  if (scanf("%d", &game.minBet) != 1 || game.minBet < 1 || game.minBet > STARTING_CHIPS) {
    printf("Invalid minimum bet. Please enter a number between 1 and %d.\n", STARTING_CHIPS);
    exit(1);
  }
  for (int i = 0; i < game.numPlayers; i++) {
    game.players[i].id = i + 1;
    game.players[i].chips = STARTING_CHIPS;
  }
  game.round = 1;

  initDeck();
}

void initRound() {
  shuffleDeck();
  struct Dealer dealer = {0};
  game.dealer = dealer;

  game.dealer.numCardsInHand = 3;
  for (int i = 0; i < game.dealer.numCardsInHand; i++) {
    game.dealer.hand[i] = getCard();
  }

  for (int i = 0; i < game.numPlayers; i++) {
    for (int j = 0; j < 2; j++)
      game.players[i].hand[j] = getCard();
    game.currentPlayer = 0;
    game.players[i].numCardsInHand = 2;
    game.players[i].hasFolded = 0;
    game.players[i].toSpend = 0;
    game.players[i].hasBet = 0;
  }

  game.activePlayers = game.numPlayers;
  game.turn = 1;
  game.pot = 0;
  game.highestBet = 0;
  game.currentPlayer = 0;
  game.currentWinner = 0;
  game.allInPlayers = 0;
  game.firstToBet = 0;
}

int progressRound() {
  printf("Round %d ended\n", game.round);

  game.players[game.currentWinner - 1].chips += game.pot;
  game.pot = 0;

  // REMOVE PLAYERS WHO WENT BANKRUPT
  for (int i = 0; i < game.numPlayers; i++) {
    if (game.players[i].chips <= 0) {
      printf("> Player %d went bankrupt\n", game.players[i].id);

      for (int j = i; j < game.numPlayers - 1; j++) {
        game.players[j] = game.players[j + 1];
      }

      game.numPlayers--;
      i--;
    }
  }

  if (game.numPlayers <= 1) { // END GAME IF 1 PLAYER IS LEFT
    return 0;
  }

  for (int i = 0; i < game.numPlayers; i++) {
    if (game.players[i].id != i + 1) {
       printf("> Player %d is now Player %d\n", game.players[i].id, i + 1);
       game.players[i].id = i + 1;
    }
  }

  // CHECK IF PLAYERS WANT TO CONTINUE
  if (game.numPlayers >= MAX_PLAYERS) {
    printf("Do you want to continue playing? (1 for Yes, 2 for No, 4 to Remove a player): ");
  } else if (game.numPlayers <= MIN_PLAYERS) {
    printf("Do you want to continue playing? (1 for Yes, 2 for No, 3 to Add another player): ");
  } else if (game.numPlayers > MIN_PLAYERS && game.numPlayers < MAX_PLAYERS) {
    printf("Do you want to continue playing? (1 for Yes, 2 for No, 3 to Add another player, 4 to Remove a player): ");
  }

  int choice;
  while (scanf("%d", &choice) != 1 || choice < 1 || choice > 4 || (choice == 3 && game.numPlayers >= MAX_PLAYERS) || (choice == 4 && game.numPlayers <= MIN_PLAYERS)) {
    if (game.numPlayers >= MAX_PLAYERS) {
      printf("Invalid choice. Please enter 1 for Yes or 2 for No.\n");
    } else if (game.numPlayers <= MIN_PLAYERS) {
      printf("Invalid choice. Please enter 1 for Yes, 2 for No, or 3 to Add another player.\n");
    } else {
      printf("Invalid choice. Please enter 1 for Yes, 2 for No, 3 to Add another player, or 4 to Remove a player.\n");
    }
    while (getchar() != '\n'); // Clear input buffer
  }

  // HANDLE CHOICE
  if (choice == 2) {
    return 0;
  } 
  else if (choice == 3 && game.numPlayers < MAX_PLAYERS) {
    if (game.numPlayers >= MAX_PLAYERS) {
      return 1;
    }
    printf("How many players do you want to add? (up to %d): ", MAX_PLAYERS - game.numPlayers);
    int toAdd;
    while (scanf("%d", &toAdd) != 1 || toAdd < 1 || game.numPlayers + toAdd > MAX_PLAYERS) {
      printf("Invalid number of players. Please enter a number between 1 and %d.\n", MAX_PLAYERS - game.numPlayers);
      while (getchar() != '\n');
    }
    for (int i = game.numPlayers; i < game.numPlayers + toAdd; ++i) {
      game.players[i].id = i + 1;
      game.players[i].chips = STARTING_CHIPS;
    }
    game.numPlayers += toAdd;

    clear_terminal();
    printf("Added %d player(s). Total players: %d\n \n", toAdd, game.numPlayers);
    return progressRound();
  }
  else if (choice == 4 && game.numPlayers > MIN_PLAYERS) {
    if (game.numPlayers <= MIN_PLAYERS) {
      return 1;
    }
    printf("How many players do you want to remove? (up to %d): ", game.numPlayers - MIN_PLAYERS);
    int toRemove;
    while (scanf("%d", &toRemove) != 1 || toRemove < 1 || game.numPlayers - toRemove < MIN_PLAYERS) {
      printf("Invalid number of players. Please enter a number between 1 and %d.\n", game.numPlayers - MIN_PLAYERS);
      while (getchar() != '\n');
    }
    for (int i = 0; i < toRemove; i++) {
      printf("Enter the ID of player to remove (1-%d): ", game.numPlayers);
      int idToRemove;
      while (scanf("%d", &idToRemove) != 1 || idToRemove < 1 || idToRemove > game.numPlayers) {
        printf("Invalid player ID. Please enter a number between 1 and %d.\n", game.numPlayers);
        while (getchar() != '\n');
      }
      if (idToRemove == game.numPlayers) {
        game.numPlayers--;
        printf("> Removed Player %d.\n", idToRemove);
      } 
      else {
        for (int j = idToRemove - 1; j < game.numPlayers - 1; j++) {
          printf("> Player %d is now Player %d\n",j + 2, j + 1);
          game.players[j] = game.players[j + 1];
          game.players[j].id = j + 1; // Update player ID
        }
        game.numPlayers--;
      }
    }
    char buffer[20];

    printf("Confirm you understand these changes (Any input): ");
    scanf("%s", buffer);
    clear_terminal();
    printf("Removed %d player(s). Total players: %d\n \n", toRemove, game.numPlayers);
    return progressRound();
  }

  // PROGRESS ROUND
  game.round++;

  return 1;
}

void progressTurn() {
  game.activePlayers = 0;
  game.allInPlayers = 0;

  for (int i = 0; i < game.numPlayers; i++) {
    if (!game.players[i].hasFolded) {
      game.activePlayers++;
    }
  }

  game.currentPlayer++; // Move to the next player's turn
  
  if (game.currentPlayer >= game.numPlayers) { // turn pass
    game.currentPlayer = 0;
    game.firstToBet = 0;
    game.highestBet = 0;
    game.turn++;

    if (game.dealer.numCardsInHand < 5) { // deal dealer card
      game.dealer.hand[game.dealer.numCardsInHand] = getCard();
      game.dealer.numCardsInHand++;
    }

    for (int i = 0; i < game.numPlayers; i++) {
      if (game.players[i].toSpend > 0) { // deduct from each turn pass
        game.pot += game.players[i].toSpend;
        game.players[i].chips -= game.players[i].toSpend;

        if (game.players[i].chips <= 0) {
          game.allInPlayers++;
        }

        game.players[i].toSpend = 0;
      }
      if (game.players[i].numCardsInHand < 5) { // deal player cards
        game.players[i].hand[game.players[i].numCardsInHand] = getCard();
        game.players[i].numCardsInHand++;
      }
    }
  }
}

int determineWinner() {
  struct Rank bestRank = {0, 0, 0};
  int winnerId = 0;

  for (int i = 0; i < game.numPlayers; i++) {
    if (!game.players[i].hasFolded) {
      struct Rank playerRank = evaluatePlayerHand(&game.players[i], &game.dealer);
      if (playerRank.rankValue > bestRank.rankValue || 
          (playerRank.rankValue == bestRank.rankValue && playerRank.rankFace > bestRank.rankFace) || 
          (playerRank.rankValue == bestRank.rankValue && playerRank.rankFace == bestRank.rankFace && playerRank.rankSuit > bestRank.rankSuit)) {
        bestRank.rankValue = playerRank.rankValue;
        bestRank.rankFace = playerRank.rankFace;
        bestRank.rankSuit = playerRank.rankSuit;
        winnerId = game.players[i].id;
      }
    }
  }
  return winnerId;
}

// DECK RELATED FUNCTIONS

void initDeck() {
  int i = 0;

  for (int suit = 0; suit < 4; suit++) {
    for (int face = 0; face < 13; face++) {
      deck.dealtCards[i].face = face;
      deck.dealtCards[i].suit = suit;
      i++;
    }
  }

  deck.numCardsDealt = 0;
}

void shuffleDeck() {
  for (int i = 51; i > 0; i--) {
    int j = rand() % (i + 1);

    struct Card temp = deck.dealtCards[i];
    deck.dealtCards[i] = deck.dealtCards[j];
    deck.dealtCards[j] = temp;
  }

  deck.numCardsDealt = 0;
}

struct Card getCard() {
  if (deck.numCardsDealt >= 52) {
    fprintf(stderr, "Deck is empty\n");
    exit(EXIT_FAILURE);
  }
  return deck.dealtCards[deck.numCardsDealt++];
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

int playerConfirmation() {
  char buffer[20];

  clear_terminal();
  printf("Confirm you are player %d (Any input): ", game.currentPlayer + 1);
  scanf("%s", buffer);
  clear_terminal();

  return 0;
}

int playerAction() {
  struct Player *currentPlayer = &game.players[game.currentPlayer]; // utility variable

  printf("[Turn %d] Player %d's action. Chips: %d\n", game.turn, currentPlayer->id, currentPlayer->chips);

  printf("Game Status (Pot: %d):\n", game.pot);
  printStatus();

  printf("Dealer's hand:\n");
  printHand(game.dealer.hand, game.dealer.numCardsInHand);

  printf("Your hand (%s):\n", rankNames[evaluatePlayerHand(currentPlayer, &game.dealer).rankValue - 1]);
  printHand(currentPlayer->hand, currentPlayer->numCardsInHand);

  printf("Choose an action (1 to Check/Call 2 to Fold 3 to Bet/Raise): ");
  
  int action;
  while (scanf("%d", &action) != 1 || action < 1 || action > 3) {
    printf("Invalid action.\n");
    while (getchar() != '\n'); // Clear input buffer
  }
  return action;
}

void printHand(struct Card hand[], int numCards) {
  const char *RED = "\x1b[31m";
  const char *BLACK = "\x1b[90m";
  const char *RESET = "\x1b[0;0m";

  for (int i = 0; i < numCards; i++) {
    if (hand[i].suit == 1 || hand[i].suit == 2) {
      printf(RED);
    } else {
      printf(BLACK);
    }
    printf("  %s of %s\n", faceNames[hand[i].face], suitNames[hand[i].suit]);
    printf(RESET);
  }
}

void printStatus() {
  const char *BET = "\x1b[0;0m";
  const char *FOLD = "\x1b[2;37m";
  const char *CHECK = "\x1b[0;0m";
  const char *ALLIN = "\x1b[1;91m";
  const char *RESET = "\x1b[0;0m";

  for (int i = 0; i < game.numPlayers; i++) {
    struct Player currentPlayer = game.players[i];

    int id = currentPlayer.id;
    int chips = currentPlayer.chips - currentPlayer.toSpend;
    int bet = game.highestBet;

    if (currentPlayer.hasFolded) {
      printf(FOLD);
      printf("  Player %d (Chips: %d) folded.\n", id, chips);
    } else if (id == game.firstToBet) {
      if (chips == 0) {
        printf(ALLIN);
        printf("> Player %d is ALL-IN!\n", id);
      } else {
        printf(BET);
        printf("> Player %d (Chips: %d) bet %d chips!\n", id, chips, bet);
      }
    } else {
      printf(CHECK);
      printf("  Player %d (Chips: %d)\n", id, chips);
    }
    printf(RESET);
  }
}

void clear_terminal() {
  const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
  printf("%s", CLEAR_SCREEN_ANSI);
}