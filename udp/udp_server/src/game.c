
#include <syslog.h> // syslog()
#include <string.h> // strlen()
#include <unistd.h> // gethostname(), write()
#include <stdlib.h>
#include "../includes/definitions.h"
#include "../includes/game.h"

char *word[] = {
# include "../words"
		};

extern time_t time();

void play_hangman(int in, int out, struct Address client, GameSession* gameSession) {

	printf("Playing game with %s\n", gameSession->strUsername);
	char * whole_word, part_word[MAXLEN], guess[MAXLEN], outbuf[MAXLEN];

	int lives = MAX_LIVES;
	//int game_state = 'I'; //I = Incomplete
	gameSession->cGameState = 'I';
	int i, good_guess, word_length;
	char hostname[MAXLEN];

	socklen_t iClientSize;
	iClientSize = sizeof(client.m_sAddress);

	gethostname(hostname, MAXLEN);
	printf("sending confirmation message to the client..\n");
	sprintf(outbuf, "Playing hangman on host %s with %s:", hostname, gameSession->strUsername);
	sendto(out, outbuf, strlen(outbuf) + 1, 0, (struct sockaddr*) &client.m_sAddress, sizeof(client.m_sAddress));

	/* Pick a word at random from the list */
	printf("picking a random word...\n");
	srand((int) time((long *) 0)); /* randomize the seed */

	whole_word = word[rand() % NUM_OF_WORDS];
	word_length = strlen(whole_word);
	syslog(LOG_USER | LOG_INFO, "server chose hangman word %s", whole_word);

	/* No letters are guessed Initially */
	for (i = 0; i < word_length; i++)
		part_word[i] = '-';

	part_word[i] = '\0';

	printf("sending game status to client..\n");
	sprintf(outbuf, "%s %d", part_word, lives);
	sendto(out, outbuf, strlen(outbuf) + 1, 0, (struct sockaddr*) &client.m_sAddress, sizeof(client.m_sAddress));

	while (gameSession->cGameState == 'I')
	/* Get a letter from player guess */
	{
		printf("waiting for guess from the client...\n");
		recvfrom(in, guess, MAXLEN, 0, (struct sockaddr*) &client.m_sAddress, &iClientSize);
		//write(1, guess, sizeof(guess));
		/*
		while (read(in, guess, MAXLEN) < 0) {
			if (errno != EINTR)
				exit(4);
			printf("re-read the startin \n");
		}
		*/
		good_guess = 0;
		for (i = 0; i < word_length; i++) {
			if (guess[0] == whole_word[i]) {
				good_guess = 1;
				part_word[i] = whole_word[i];
			}
		}
		if (!good_guess)
			lives--;
		if (strcmp(whole_word, part_word) == 0)
			gameSession->cGameState = 'W';
		else if (lives == 0) {
			//game_state = 'L';
			gameSession->cGameState = 'L';
			strcpy(part_word, whole_word);
		}

		printf("sending game state to the client...\n");
		sprintf(outbuf, "%s %d", part_word, lives);
		sendto(out, outbuf, strlen(outbuf) + 1, 0, (struct sockaddr*) &client.m_sAddress, sizeof(client.m_sAddress));
	}
}


