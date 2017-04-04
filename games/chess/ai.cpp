// AI
// This is where you build your AI

#include "ai.hpp"

// You can add #includes here for your AI.

namespace cpp_client
{

namespace chess
{

/// <summary>
/// This returns your AI's name to the game server.
/// Replace the string name.
/// </summary>
/// <returns>The name of your AI.</returns>
std::string AI::get_name() const
{
    // REPLACE WITH YOUR TEAM NAME!
    return "Anthony Nguyen";
}

/// <summary>
/// This is automatically called when the game first starts, once the game objects are created
/// </summary>
void AI::start()
{
	// This is a good place to initialize any variables
	srand(time(NULL));

	//initialize state
	if (game->current_player->color == "White")
		state.turn = 0;
	else
		state.turn = 1;

	state.halfmove_clock = game->turns_to_draw;

	for (int j = 0; j < 2; j++)
	{
		state.players[j].rank_direction = game->players[j]->rank_direction;
		for (int i = 0; i < 16; i++)
		{
			state.players[j].pieces[i].captured = game->players[j]->pieces[i]->captured;
			state.players[j].pieces[i].type = game->players[j]->pieces[i]->type;
			state.players[j].pieces[i].file = game->players[j]->pieces[i]->file;
			state.players[j].pieces[i].rank = game->players[j]->pieces[i]->rank;
		}
	}

	std::string castle;
	std::stringstream fenstring(game->fen);
	fenstring.ignore(256, ' ');
	fenstring.ignore(256, ' ');
	fenstring >> castle;

	if (castle.find('K') != castle.npos)
	{
		state.players[0].k_castle = true;
	}
	if (castle.find('Q') != castle.npos)
	{
		state.players[0].q_castle = true;
	}
	if (castle.find('k') != castle.npos)
	{
		state.players[1].k_castle = true;
	}
	if (castle.find('q') != castle.npos)
	{
		state.players[1].q_castle = true;
	}

	fenstring >> castle;

	if (castle != "-")
	{
		state.en_pass_file = castle[0];
		std::stringstream temp;
		temp << castle[1];
		temp >> state.en_pass_rank;
	}
}

/// <summary>
/// This is automatically called the game (or anything in it) updates
/// </summary>
void AI::game_updated()
{
    // If a function you call triggers an update this will be called before it returns.
}

/// <summary>
/// This is automatically called when the game ends.
/// </summary>
/// <param name="won">true if you won, false otherwise</param>
/// <param name="reason">An explanation for why you either won or lost</param>
void AI::ended(bool won, const std::string& reason)
{
    // You can do any cleanup of your AI here.  The program ends when this function returns.
}

/// <summary>
/// This is called every time it is this AI.player's turn.
/// </summary>
/// <returns>Represents if you want to end your turn. True means end your turn, False means to keep your turn going and re-call this function.</returns>
bool AI::run_turn()
{
    if(game->moves.size() > 0)
    {
		int index;
		for (int i = 0; i < 16; i++)
		{
			if (player->opponent->pieces[i]->id == game->moves[game->moves.size() - 1]->piece->id)
			{
				index = i;
				break;
			}
		}

		//update our current state based on opponents last move
		Action lastMove;
		if(game->moves[game->moves.size() - 1]->promotion == "")
			lastMove.update(index, game->moves[game->moves.size() - 1]->to_file, game->moves[game->moves.size() - 1]->to_rank);
		else
			lastMove.update(index, game->moves[game->moves.size() - 1]->to_file, game->moves[game->moves.size() - 1]->to_rank, game->moves[game->moves.size() - 1]->promotion);
		state = results(state, lastMove);
    }

	//determine our best move
	Action myMove = tl_id_dlmm(state);

	//update state based on current move
	state = results(state, myMove);

	//make our move on the server
	if(myMove.promotion == "0")
		player->pieces[myMove.piece]->move(myMove.file, myMove.rank);
	else
		player->pieces[myMove.piece]->move(myMove.file, myMove.rank, myMove.promotion);

    return true;
}

// You can add additional methods here for your AI to call

int AI::ctoi(const std::string& c)
{
	int i;
	switch (c[0])
	{
	case 'a':
		i = 1;
		break;
	case 'b':
		i = 2;
		break;
	case 'c':
		i = 3;
		break;
	case 'd':
		i = 4;
		break;
	case 'e':
		i = 5;
		break;
	case 'f':
		i = 6;
		break;
	case 'g':
		i = 7;
		break;
	case 'h':
		i = 8;
		break;
	default:
		i = 0;
	}

	return i;
}

void AI::actionSet(const State& s, std::vector<Action>& actions, const bool& checkFlag)
{
	Action tmp;
	const Players* p = &(s.players[s.turn]);

	int board[8][8];
	for (int rank = 1; rank < 9; rank++)
	{
		for (int file_offset = 0; file_offset < 8; file_offset++)
		{
			std::string file(1, 'a' + file_offset); // start at a, with with file offset increasing the char;
			bool found = false;
			bool owner;
			for (int j = 0; j < 2; j++)
			{
				for (int i = 0; i < s.players[j].pieces.size(); i++)
				{
					if (s.players[j].pieces[i].file == file && s.players[j].pieces[i].rank == rank)
					{
						found = true;
						owner = j;
						break;
					}
				}
				if (found == true)
					break;
			}
			board[file_offset][rank-1] = -1;
			if (found == true)
			{
				if (owner == 0)
					board[file_offset][rank-1] = 0;
				if (owner == 1)
					board[file_offset][rank-1] = 1;
			}	
		}
	}

	for (int i = 0; i < p->pieces.size(); i++)
	{
		std::string toFile = "0";
		int toRank;
		char space;


		if (p->pieces[i].type == "Pawn")
		{
			toFile = p->pieces[i].file;
			toRank = p->pieces[i].rank + p->rank_direction;
			space = checkMove(s, toFile, toRank, board);
			if (space == 'n')
			{
				if ((s.turn == 0 && toRank == 8)||(s.turn == 1 && toRank == 1))
				{
					addAction(s, actions, tmp, checkFlag, i, toFile, toRank, "Queen");
					addAction(s, actions, tmp, checkFlag, i, toFile, toRank, "Rook");
					addAction(s, actions, tmp, checkFlag, i, toFile, toRank, "Bishop");
					addAction(s, actions, tmp, checkFlag, i, toFile, toRank, "Knight");
				}
				else
					addAction(s, actions, tmp, checkFlag, i, toFile, toRank);

				toRank = toRank + p->rank_direction;
				space = checkMove(s, toFile, toRank, board);
				if ((space == 'n')&&((s.turn == 0 && p->pieces[i].rank == 2)||(s.turn == 1 && p->pieces[i].rank == 7)))
					addAction(s, actions, tmp, checkFlag, i, toFile, toRank);
			}

			toRank = p->pieces[i].rank + p->rank_direction;
			for (int j = 0; j < 2; j++)
			{
				if (j == 0)
					toFile[0] = p->pieces[i].file[0] + 1;
				else
					toFile[0] = p->pieces[i].file[0] - 1;

				space = checkMove(s, toFile, toRank, board);
				if (space == 'o' || (toRank == s.en_pass_rank && toFile == s.en_pass_file))
				{
					if ((s.turn == 0 && toRank == 8) || (s.turn == 1 && toRank == 1))
					{
						addAction(s, actions, tmp, checkFlag, i, toFile, toRank, "Queen");
						addAction(s, actions, tmp, checkFlag, i, toFile, toRank, "Rook");
						addAction(s, actions, tmp, checkFlag, i, toFile, toRank, "Bishop");
						addAction(s, actions, tmp, checkFlag, i, toFile, toRank, "Knight");
					}
					else
						addAction(s, actions, tmp, checkFlag, i, toFile, toRank);
				}
			}

		}


		if (p->pieces[i].type == "Knight")
		{
			toFile[0] = p->pieces[i].file[0] - 2;
			toRank = p->pieces[i].rank + 1;
			moveAction(s, board, actions, tmp, checkFlag, i, toFile, toRank);

			toRank = p->pieces[i].rank - 1;
			moveAction(s, board, actions, tmp, checkFlag, i, toFile, toRank);

			toFile[0] = p->pieces[i].file[0] + 2;
			moveAction(s, board, actions, tmp, checkFlag, i, toFile, toRank);

			toRank = p->pieces[i].rank + 1;
			moveAction(s, board, actions, tmp, checkFlag, i, toFile, toRank);

			toFile[0] = p->pieces[i].file[0] + 1;
			toRank = p->pieces[i].rank + 2;
			moveAction(s, board, actions, tmp, checkFlag, i, toFile, toRank);

			toRank = p->pieces[i].rank - 2;
			moveAction(s, board, actions, tmp, checkFlag, i, toFile, toRank);

			toFile[0] = p->pieces[i].file[0] - 1;
			moveAction(s, board, actions, tmp, checkFlag, i, toFile, toRank);

			toRank = p->pieces[i].rank + 2;
			moveAction(s, board, actions, tmp, checkFlag, i, toFile, toRank);
		}


		if (p->pieces[i].type == "King")
		{
			//if not check
			State check = s;
			check.turn = !check.turn;

			toRank = p->pieces[i].rank;

			bool cc;

			if (checkFlag == 1)
				cc = false;
			else
				cc = checkCheck(check);

			if (cc == false)
			{
				//kingside castling
				toFile[0] = p->pieces[i].file[0] + 1;
				space = checkMove(s, toFile, toRank, board);

				toFile[0]++;
				char space2;
				space2 = checkMove(s, toFile, toRank, board);
				if (p->k_castle == true && space == 'n' && space2 == 'n')
					addAction(s, actions, tmp, checkFlag, i, toFile, toRank);

				//queenside castling
				toFile[0] = p->pieces[i].file[0] - 1;
				space = checkMove(s, toFile, toRank, board);

				toFile[0]--;
				space2 = checkMove(s, toFile, toRank, board);

				toFile[0]--;
				char space3;
				space3 = checkMove(s, toFile, toRank, board);
				if (p->q_castle == true && space == 'n' && space2 == 'n' && space2 == 'n')
				{
					toFile[0]++;
					addAction(s, actions, tmp, checkFlag, i, toFile, toRank);
				}
			}

			//right
			toFile[0] = p->pieces[i].file[0] + 1;
			moveAction(s, board, actions, tmp, checkFlag, i, toFile, toRank);

			//top right
			toRank++;
			moveAction(s, board, actions, tmp, checkFlag, i, toFile, toRank);

			//bottom right
			toRank = toRank - 2;
			moveAction(s, board, actions, tmp, checkFlag, i, toFile, toRank);

			//bottom
			toFile[0]--;
			moveAction(s, board, actions, tmp, checkFlag, i, toFile, toRank);

			//top
			toRank = toRank + 2;
			moveAction(s, board, actions, tmp, checkFlag, i, toFile, toRank);

			//top left
			toFile[0]--;
			moveAction(s, board, actions, tmp, checkFlag, i, toFile, toRank);

			//left
			toRank--;
			moveAction(s, board, actions, tmp, checkFlag, i, toFile, toRank);

			//bottom left
			toRank--;
			moveAction(s, board, actions, tmp, checkFlag, i, toFile, toRank);
		}


		if (p->pieces[i].type == "Queen")
		{
			//moves are identical to rook
			rookActions(s, p->pieces[i], board, actions, tmp, checkFlag, i);

			//move are identical to bishop
			bishopActions(s, p->pieces[i], board, actions, tmp, checkFlag, i);
		}


		if (p->pieces[i].type == "Rook")
			rookActions(s, p->pieces[i], board, actions, tmp, checkFlag, i);


		if (p->pieces[i].type == "Bishop")
			bishopActions(s, p->pieces[i], board, actions, tmp, checkFlag, i);
	}

	return;
}

State AI::results(const State& parent, const Action& a)
{
	State child = parent;
	Players* p = &(child.players[child.turn]);
	Players* o = &(child.players[!child.turn]);

	bool capture = false;
	bool pawnMove = false;
	bool promotion = false;

	for (int i = 0; i < o->pieces.size(); i++)
	{
		if (o->pieces[i].file == a.file && o->pieces[i].rank == a.rank)
		{
			o->pieces[i].captured = true;
			o->pieces.erase(o->pieces.begin() + i);
			capture = true;
			break;
		}
	}

	if (p->pieces[a.piece].type == "Rook" && p->pieces[a.piece].file == "h")
		p->k_castle = false;

	if (p->pieces[a.piece].type == "Rook" && p->pieces[a.piece].file == "a")
		p->q_castle = false;

	if (p->pieces[a.piece].type == "King")
	{
		p->q_castle = false;
		p->k_castle = false;

		//castling
		if (p->pieces[a.piece].file[0] + 2 == a.file[0])
		{
			for (int i = 0; i < p->pieces.size(); i++)
			{
				if (p->pieces[i].type == "Rook" && p->pieces[i].file == "h")
				{
					p->pieces[i].file = "f";
					break;
				}
			}
		}

		if (p->pieces[a.piece].file[0] - 2 == a.file[0])
		{
			for (int i = 0; i < p->pieces.size(); i++)
			{
				if (p->pieces[i].type == "Rook" && p->pieces[i].file == "a")
				{
					p->pieces[i].file = "d";
					break;
				}
			}
		}
	}

	if (p->pieces[a.piece].type == "Pawn")
	{
		pawnMove = true;

		if (a.file == child.en_pass_file && a.rank == child.en_pass_rank)
		{
			for (int i = 0; i < o->pieces.size(); i++)
			{
				if (o->pieces[i].file == a.file
					&& o->pieces[i].rank == a.rank + o->rank_direction)
				{
					o->pieces[i].captured = true;
					o->pieces.erase(o->pieces.begin() + i);
					capture = true;
					break;
				}
			}
		}

		if (abs(p->pieces[a.piece].rank - a.rank) == 2)
		{
			child.en_pass_file = a.file;
			child.en_pass_rank = p->pieces[a.piece].rank + p->rank_direction;
		}
		else //issue here
		{
			child.en_pass_file = "0";
			child.en_pass_rank = 0;
		}
	}
	else //issue here
	{
		child.en_pass_file = "0";
		child.en_pass_rank = 0;
	}

	RepDraw currentmove;
	currentmove.sfile = p->pieces[a.piece].file;
	currentmove.srank = p->pieces[a.piece].rank;
	currentmove.efile = a.file;
	currentmove.erank = a.rank;

	p->pieces[a.piece].file = a.file;
	p->pieces[a.piece].rank = a.rank;
	if (a.promotion != "0")
	{
		p->pieces[a.piece].type = a.promotion;
		promotion = true;
	}

	child.turn = !child.turn;

	if (capture == true || pawnMove == true)
		child.halfmove_clock == 100;
	else
		child.halfmove_clock--;

	currentmove.capture = capture;
	currentmove.pawnMove = pawnMove;
	currentmove.promotion = promotion;

	child.repDraw.push_back(currentmove);
	if (child.repDraw.size() == 9)
		child.repDraw.pop_front();

	return child;
}

char AI::checkMove(const State& s, const std::string& file, const int& rank, int board[][8])
{
	int fileNum;
	switch (file[0])
	{
	case 'a':
		fileNum = 1;
		break;
	case 'b':
		fileNum = 2;
		break;
	case 'c':
		fileNum = 3;
		break;
	case 'd':
		fileNum = 4;
		break;
	case 'e':
		fileNum = 5;
		break;
	case 'f':
		fileNum = 6;
		break;
	case 'g':
		fileNum = 7;
		break;
	case 'h':
		fileNum = 8;
		break;
	default:
		return 'b';
	}

	if (rank <= 0 || rank >= 9)
		return 'b';

	int space = board[fileNum - 1][rank - 1];
	if (space == s.turn)
		return 'y';
	else if (space == -1)
		return 'n';
	else
		return 'o';
}

bool AI::checkCheck(const State& test)
{
	std::vector<Action> testActions;
	actionSet(test, testActions, 1);

	std::string king_file;
	int king_rank;

	for (int i = 0; i < test.players[!test.turn].pieces.size(); i++)
	{
		if (test.players[!test.turn].pieces[i].type == "King")
		{
			king_file = test.players[!test.turn].pieces[i].file;
			king_rank = test.players[!test.turn].pieces[i].rank;
			break;
		}
	}

	for (int i = 0; i < testActions.size(); i++)
	{
		if (testActions[i].file == king_file && testActions[i].rank == king_rank)
			return true;
	}
	return false;
}

void AI::addAction(const State& s, std::vector<Action>& actions, Action& tmp, const bool& checkFlag, const int& i, const std::string& toFile, const int& toRank, const std::string& promotion)
{
	tmp.update(i, toFile, toRank, promotion);
	if (checkFlag == 0)
	{
		if (checkCheck(results(s, tmp)) == false)
			actions.push_back(tmp);
	}
	else
		actions.push_back(tmp);
	return;
}

void AI::moveAction(const State& s, int board[][8], std::vector<Action>& actions, Action& tmp, const bool& checkFlag, const int& i, const std::string& toFile, const int& toRank, const std::string& promotion)
{
	char space = checkMove(s, toFile, toRank, board);
	if (space == 'o' || space == 'n')
		addAction(s, actions, tmp, checkFlag, i, toFile, toRank);
}

bool AI::moveActionQRB(const State& s, int board[][8], std::vector<Action>& actions, Action& tmp, const bool& checkFlag, const int& i, const std::string& toFile, const int& toRank, const std::string& promotion)
{
	char space = checkMove(s, toFile, toRank, board);
	if (space == 'y')
		return true;
	else if (space == 'n')
		addAction(s, actions, tmp, checkFlag, i, toFile, toRank);
	else if (space == 'o')
	{
		addAction(s, actions, tmp, checkFlag, i, toFile, toRank);
		return true;
	}
	return false;
}

void AI::rookActions(const State& s, const Pieces& p, int board[][8], std::vector<Action>& actions, Action& tmp, const bool& checkFlag, const int& i, const std::string& promotion)
{
	//going up from rook
	std::string toFile = p.file;
	int toRank;
	for (toRank = p.rank + 1; toRank < 9; toRank++)
	{
		if (moveActionQRB(s, board, actions, tmp, checkFlag, i, toFile, toRank) == true)
			break;
	}
	//going down from rook
	for (toRank = p.rank - 1; toRank > 0; toRank--)
	{
		if (moveActionQRB(s, board, actions, tmp, checkFlag, i, toFile, toRank) == true)
			break;
	}
	//going right from rook
	toRank = p.rank;
	for (toFile[0] = p.file[0] + 1; toFile[0] < 'i'; toFile[0]++)
	{
		if (moveActionQRB(s, board, actions, tmp, checkFlag, i, toFile, toRank) == true)
			break;
	}
	//going left from rook
	for (toFile[0] = p.file[0] - 1; toFile[0] > ('a' - 1); toFile[0]--)
	{
		if (moveActionQRB(s, board, actions, tmp, checkFlag, i, toFile, toRank) == true)
			break;
	}
}

void AI::bishopActions(const State& s, const Pieces& p, int board[][8], std::vector<Action>& actions, Action& tmp, const bool& checkFlag, const int& i, const std::string& promotion)
{
	std::string toFile = "0";
	int toRank;

	//moving up-right from bishop
	toFile[0] = p.file[0] + 1;
	for (toRank = p.rank + 1; toRank < 9; toRank++)
	{
		if (toFile[0] >= 'i')
			break;

		if (moveActionQRB(s, board, actions, tmp, checkFlag, i, toFile, toRank) == true)
			break;
		toFile[0]++;
	}
	//moving down-right from bishop
	toFile[0] = p.file[0] + 1;
	for (toRank = p.rank - 1; toRank > 0; toRank--)
	{
		if (toFile[0] >= 'i')
			break;

		if (moveActionQRB(s, board, actions, tmp, checkFlag, i, toFile, toRank) == true)
			break;
		toFile[0]++;
	}
	//moving up-left from bishop
	toFile[0] = p.file[0] - 1;
	for (toRank = p.rank + 1; toRank < 9; toRank++)
	{
		if (toFile[0] <= ('a' - 1))
			break;

		if (moveActionQRB(s, board, actions, tmp, checkFlag, i, toFile, toRank) == true)
			break;
		toFile[0]--;
	}
	//moving down-left from bishop
	toFile[0] = p.file[0] - 1;
	for (toRank = p.rank - 1; toRank > 0; toRank--)
	{
		if (toFile[0] <= ('a' - 1))
			break;

		if (moveActionQRB(s, board, actions, tmp, checkFlag, i, toFile, toRank) == true)
			break;
		toFile[0]--;
	}
}

Action AI::tl_id_dlmm(const State& s)
{
	Action bestMove;
	int i = 1;
	int n = 300; //max number of expected moves for chess game
	float constantTime = (15.0/n)*60;
	float compareTime;
	float takenTime;
	clock_t startTime = clock();

	while(1)
	{
		bestMove = dlmm(s, i, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
		clock_t checkTime = clock();
		takenTime = double(checkTime-startTime)/CLOCKS_PER_SEC;
		if (player->opponent->time_remaining > player->time_remaining)
			compareTime = constantTime / 2.0;
		else
			compareTime = constantTime;
		if (takenTime >= compareTime)
			break;
		i++;
	}
	return bestMove;
}

Action AI::dlmm(const State& s, const int& d, int alpha, int beta)
{
	std::vector<Action> currentActions;
	actionSet(s, currentActions, 0);

	//random action order
	int randNum = std::rand() % currentActions.size();
	int max = minV(results(s, currentActions[randNum]), d - 1, alpha, beta);
	alpha = max;

	Action index = currentActions[randNum];
	currentActions.erase(currentActions.begin()+randNum);
	int tmp;

	while (currentActions.size() != 0)
	{
		randNum = std::rand() % currentActions.size();
		tmp = minV(results(s, currentActions[randNum]), d - 1, alpha, beta);
		if (tmp > max)
		{
			alpha = tmp;
			index = currentActions[randNum];
			max = tmp;
		}
		currentActions.erase(currentActions.begin() + randNum);
	}

	return index;
}

int AI::maxV(const State& s, const int& d, int alpha, int beta)
{
	std::vector<Action> currentActions;
	actionSet(s, currentActions, 0);

	if (d == 0)
		return eval(s);

	if (currentActions.size() == 0)
		return eval(s);

	//random action order
	int randNum;
	int max = std::numeric_limits<int>::min();
	int tmp;
	while (currentActions.size() != 0)
	{
		randNum = std::rand() % currentActions.size();
		tmp = minV(results(s, currentActions[randNum]), d - 1, alpha, beta);
		currentActions.erase(currentActions.begin() + randNum);

		//Fail high on MaxV, Prune
		if (tmp >= beta)
			return tmp;

		if (tmp > max)
		{
			if(tmp > alpha)
				alpha = tmp;
			max = tmp;
		}
	}

	return max;
}

int AI::minV(const State& s, const int& d, int alpha, int beta)
{
	std::vector<Action> currentActions;
	actionSet(s, currentActions, 0);

	if (d == 0)
		return eval(s);

	if (currentActions.size() == 0)
		return eval(s);

	//random action order
	int randNum;
	int min = std::numeric_limits<int>::max();
	int tmp;
	while (currentActions.size() != 0)
	{
		randNum = std::rand() % currentActions.size();
		tmp = maxV(results(s, currentActions[randNum]), d - 1, alpha, beta);
		currentActions.erase(currentActions.begin() + randNum);

		//Fail Low on MinV, Prune
		if (tmp <= alpha)
			return tmp;

		if (tmp < min)
		{
			if (tmp < beta)
				beta = tmp;
			min = tmp;
		}
	}

	return min;
}

int AI::eval(const State& s)
{
	bool max;
	bool min;
	bool player;

	if (game->current_player->color == "White")
	{
		max = 0;
		min = 1;
	}
	else
	{
		min = 0;
		max = 1;
	}

	int maxValue = 0;
	int minValue = 0;
	int* value;

	std::vector<std::string> piecesLeft;

	for (int j = 0; j < 2; j++)
	{
		if (j == 0)
		{
			player = max;
			value = &maxValue;
		}
		else
		{
			player = min;
			value = &minValue;
		}

		for (int i = 0; i < s.players[player].pieces.size(); i++)
		{
			piecesLeft.push_back(s.players[player].pieces[i].type);

			if (s.players[player].pieces[i].type == "Pawn")
				*value = *value + 1;
			if (s.players[player].pieces[i].type == "Knight")
				*value = *value + 3;
			if (s.players[player].pieces[i].type == "Bishop")
				*value = *value + 3;
			if (s.players[player].pieces[i].type == "Rook")
				*value = *value + 5;
			if (s.players[player].pieces[i].type == "Queen")
				*value = *value + 9;
		}
	}

	//if state is a draw, then return 0
	State check = s;
	check.turn = !check.turn;

	if (checkCheck(check) == false) 
	{
		std::vector<Action> currentActions;
		actionSet(s, currentActions, 0);

		if (currentActions.size() == 0)
		{
			return 0;
		}
	}
	//if state is a win, then return 1000
	//if state is a loss, then return -1000
	else
	{
		std::vector<Action> currentActions;
		actionSet(s, currentActions, 0);

		if (currentActions.size() == 0)
		{
			if (s.turn == min)
			{
				return 1000;
			}
			else
			{
				return -1000;
			}
		}
	}

	if (s.halfmove_clock == 0)
	{
		return 0;
	}

	bool draw = true;

	if (s.repDraw.size() == 8)
	{
		for (int i = 0; i < 8; i++)
		{
			if (s.repDraw[i].capture == true || s.repDraw[i].pawnMove == true || s.repDraw[i].promotion == true)
			{
				draw = false;
				break;
			}
		}
		if (draw == true)
		{
			for (int i = 0; i < 4; i++)
			{
				if (s.repDraw[i].sfile == s.repDraw[i + 4].sfile &&
					s.repDraw[i].efile == s.repDraw[i + 4].efile &&
					s.repDraw[i].srank == s.repDraw[i + 4].srank &&
					s.repDraw[i].erank == s.repDraw[i + 4].erank)
				{
					return 0;
				}
			}
		}
	}

	if (piecesLeft.size() == 2 && piecesLeft[0] == "King" && piecesLeft[1] == "King")
	{
		return 0;
	}

	if (piecesLeft.size() == 3)
	{
		int kingCount = 0;
		int knightCount = 0;
		int bishopCount = 0;

		for (int i = 0; i < 3; i++)
		{
			if (piecesLeft[i] == "King")
				kingCount++;
			if (piecesLeft[i] == "Knight")
				knightCount++;
			if (piecesLeft[i] == "Bishop")
				bishopCount++;
		}

		if (kingCount == 2)
		{
			if (knightCount == 1)
			{
				return 0;
			}
			else if (bishopCount == 1)
			{
				return 0;
			}
		}
	}

	if (piecesLeft.size() == 4)
	{
		int kingCount = 0;

		for (int i = 0; i < 4; i++)
		{
			if (piecesLeft[i] == "King")
				kingCount++;
		}

		if (kingCount == 2)
		{
			Pieces bMax;
			Pieces bMin;
			int bishopCount = 0;
			for (int i = 0; i < 2; i++)
			{
				if (s.players[max].pieces[i].type == "Bishop")
				{
					bMax = s.players[max].pieces[i];
					bishopCount++;
				}
			}
			for (int i = 0; i < 2; i++)
			{
				if (s.players[min].pieces[i].type == "Bishop")
				{
					bMin = s.players[min].pieces[i];
					bishopCount++;
				}
			}

			if (bishopCount == 2)
			{
				int bMaxFile = ctoi(bMax.file);
				int bMinFile = ctoi(bMin.file);

				if ((std::abs(bMax.rank - bMin.rank) + std::abs(bMaxFile - bMinFile)) % 2 == 0)
				{
					return 0;
				}
			}
		}
	}

	return maxValue - minValue;
}

} // chess

} // cpp_client
