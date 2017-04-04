#ifndef GAMES_CHESS_AI_HPP
#define GAMES_CHESS_AI_HPP

#include "impl/chess.hpp"
#include "game.hpp"
#include "game_object.hpp"
#include "move.hpp"
#include "piece.hpp"
#include "player.hpp"

#include "../../joueur/src/base_ai.hpp"
#include "../../joueur/src/attr_wrapper.hpp"

// You can add additional #includes here
#include <sstream>
#include <vector>
#include <deque>
#include <cmath>
#include <limits>
#include <time.h>

namespace cpp_client
{

namespace chess
{

class Pieces
{
public:
	bool captured = true;
	std::string type = "0";
	std::string file = "0";
	int rank = 0;

	Pieces()
	{
	}

	const Pieces& operator= (const Pieces& rhs)
	{
		if (this != &rhs)
		{
			captured = rhs.captured;
			type = rhs.type;
			file = rhs.file;
			rank = rhs.rank;
		}
		return *this;
	}

	//copy constructor
	Pieces(const Pieces& rhs)
	{
		*this = rhs;
	}
};

class Action
{
public:
	int piece;
	std::string file;
	int rank;
	std::string promotion;

	//default constructor
	Action(const int& p = 0, const std::string& f = "0", const int& r = 0, const std::string& m = "0")
	{
		piece = p;
		file = f;
		rank = r;
		promotion = m;
	}

	const Action& operator= (const Action& rhs)
	{
		if (this != &rhs)
		{
			piece = rhs.piece;
			file = rhs.file;
			rank = rhs.rank;
			promotion = rhs.promotion;
		}
		return *this;
	}

	//copy constructor
	Action(const Action& rhs)
	{
		*this = rhs;
	}

	//mutator function for easier modification of an action
	void update(const int& p, const std::string& f, const int& r, const std::string& m = "0")
	{
		piece = p;
		file = f;
		rank = r;
		promotion = m;
		return;
	}
};

class Players
{
public:
	std::vector<Pieces> pieces = std::vector<Pieces>(16);
	int rank_direction;
	bool q_castle = false;
	bool k_castle = false;

	Players()
	{
	}

	const Players& operator= (const Players& rhs)
	{
		if (this != &rhs)
		{
			//for (int i = 0; i < 16; i++)
			//{
			//	pieces[i] = rhs.pieces[i];
			//}
			pieces = rhs.pieces;
			rank_direction = rhs.rank_direction;
			q_castle = rhs.q_castle;
			k_castle = rhs.k_castle;
		}
		return *this;
	}

	//copy constructor
	Players(const Players& rhs)
	{
		*this = rhs;
	}
};

class RepDraw
{
public:
	bool capture = false;
	bool promotion = false;
	bool pawnMove = false;
	std::string sfile = "0";
	int srank = 0;
	std::string efile = "0";
	int erank = 0;

	RepDraw()
	{
	}

	const RepDraw& operator= (const RepDraw& rhs)
	{
		if (this != &rhs)
		{
			capture = rhs.capture;
			promotion = rhs.promotion;
			pawnMove = rhs.pawnMove;
			sfile = rhs.sfile;
			srank = rhs.srank;
			efile = rhs.efile;
			erank = rhs.erank;
		}
		return *this;
	}

	RepDraw(const RepDraw& rhs)
	{
		*this = rhs;
	}
};

class State
{
public:
	Players players[2]; //player 0 = white, player 1 = black
	bool turn; //0 = white's, 1 = black's
	int halfmove_clock;
	std::string en_pass_file = "0";
	int en_pass_rank = 0;
	std::deque<RepDraw> repDraw;

	State()
	{
	}

	const State& operator= (const State& rhs)
	{
		if (this != &rhs)
		{
			for (int i = 0; i < 2; i++)
			{
				players[i] = rhs.players[i];
			}
			turn = rhs.turn;
			halfmove_clock = rhs.halfmove_clock;
			en_pass_file = rhs.en_pass_file;
			en_pass_rank = rhs.en_pass_rank;
			repDraw = rhs.repDraw;
		}
		return *this;
	}

	//copy constructor
	State(const State& rhs)
	{
		*this = rhs;
	}
};

/// <summary>
/// This is the header file for building your Chess AI
/// </summary>
class AI : public Base_ai
{
public:
    /// <summary>
    /// This is a reference to the Game object itself, it contains all the information about the current game
    /// </summary>
    Game game;

    /// <summary>
    /// This is a pointer to your AI's player. This AI class is not a player, but it should command this Player.
    /// </summary>
    Player player;

    // You can add additional class variables here.
	State state;

    /// <summary>
    /// This returns your AI's name to the game server.
    /// Replace the string name.
    /// </summary>
    /// <returns>The name of your AI.</returns>
    virtual std::string get_name() const override;

    /// <summary>
    /// This is automatically called when the game first starts, once the game objects are created
    /// </summary>
    virtual void start() override;

    /// <summary>
    /// This is automatically called when the game ends.
    /// </summary>
    /// <param name="won">true if you won, false otherwise</param>
    /// <param name="reason">An explanation for why you either won or lost</param>
    virtual void ended(bool won, const std::string& reason) override;

    /// <summary>
    /// This is automatically called the game (or anything in it) updates
    /// </summary>
    virtual void game_updated() override;

    /// <summary>
    /// This is called every time it is this AI.player's turn.
    /// </summary>
    /// <returns>Represents if you want to end your turn. True means end your turn, False means to keep your turn going and re-call this function.</returns>
    bool run_turn();

    /// <summary>
    ///  Prints the current board using pretty ASCII art
    /// </summary>
    /// <remarks>
    /// Note: you can delete this function if you wish
    /// </remarks>
    void print_current_board();

    // You can add additional methods here.
	int ctoi(const std::string& c);

	void print_state(const State& s);

	void actionSet(const State& s, std::vector<Action>& actions, const bool& checkFlag);

	char checkMove(const State& s, const std::string& file, const int& rank, int board[][8]);

	State results(const State& parent, const Action& a);

	bool checkCheck(const State& test);

	void addAction(const State& s, std::vector<Action>& actions, Action& tmp, const bool& checkFlag, const int& i, const std::string& toFile, const int& toRank, const std::string& promotion = "0");

	void moveAction(const State& s, int board[][8], std::vector<Action>& actions, Action& tmp, const bool& checkFlag, const int& i, const std::string& toFile, const int& toRank, const std::string& promotion = "0");

	bool moveActionQRB(const State& s, int board[][8], std::vector<Action>& actions, Action& tmp, const bool& checkFlag, const int& i, const std::string& toFile, const int& toRank, const std::string& promotion = "0");

	void rookActions(const State& s, const Pieces& p, int board[][8], std::vector<Action>& actions, Action& tmp, const bool& checkFlag, const int& i, const std::string& promotion = "0");

	void bishopActions(const State& s, const Pieces& p, int board[][8], std::vector<Action>& actions, Action& tmp, const bool& checkFlag, const int& i, const std::string& promotion = "0");

	Action tl_id_dlmm(const State& s);

	Action dlmm(const State& s, const int& d, int alpha, int beta);

	int maxV(const State& s, const int& d, int alpha, int beta);

	int minV(const State& s, const int& d, int alpha, int beta);

	int eval(const State& s);

    // ####################
    // Don't edit these!
    // ####################
    /// \cond FALSE
    virtual std::string invoke_by_name(const std::string& name,
                                       const std::unordered_map<std::string, Any>& args) override;
    virtual void set_game(Base_game* ptr) override;
    virtual void set_player(std::shared_ptr<Base_object> obj) override;
    virtual void print_win_loss_info() override;
    /// \endcond
    // ####################
    // Don't edit these!
    // ####################

};

} // CHESS

} // cpp_client

/*std::ostream& operator<< (std::ostream& out, const cpp_client::chess::Action& a)
{
	out << a.piece << ' ' << a.file << a.rank;

	return out;
}*/

#endif // GAMES_CHESS_AI_HPP
