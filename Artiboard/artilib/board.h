#pragma once
#include <array>
#include <forward_list>
#include <list>
#include <ostream>
#include <memory>
#include "square.h"
/**
 * The framework presumes that a board game is played on a Board that contains 64 squares.
 * Each square on the board has a Piece.  There are two players, one on the south
 * Side (looking north) and one of the north Side.  Starting from from Ply zero, the
 * south player creates the first Move.
 *
 * In order to play a game, create an instance of Match, giving it a GameSpecification and a MoveChooser.
 * At the end of a Match, a PlayLine contains the moves that can be made.  The PlayLine consists
 * of a sequence of Positions.  A Position is for a Ply and it describes the Board after the move is
 * made for that Ply.
 *
 * An implementation of the GameSpecification specifies the Moves that can be made from a Position.
 * A Move consists of one or more Steps.  A Step is a smal change on the Board.
 *
 */
namespace arti {
	using std::unique_ptr;
	using std::shared_ptr;
	using std::ostream;

	/**
	 * The integral value of a Piece
	 */
	typedef char square_value_t;

	/**
	 * The value that is placed on a square
	 *
	 */
	class Piece {
		public:
			/**
			 * Marks a square as empty
			 */
			const static Piece EMPTY;
			/**
			 * Signifies that the square has no value.
			 */
			const static Piece OUT_OF_BOUNDS;
			explicit Piece(square_value_t v = EMPTY._value);
			bool is_empty() const {return _value == EMPTY._value;}			;
			bool is_out_of_bounds() const {return _value == OUT_OF_BOUNDS._value;}			;
			bool operator !=(const Piece& other) const {return _value != other._value;}			;
			bool operator ==(const Piece& other) const {return _value == other._value;}			;
			bool operator <(const Piece& other) const {return _value < other._value;}			;
			square_value_t index() const {return _value;}			;

		private:
			square_value_t _value;
		public:
			static bool is_same(const Piece& p1, const Piece& p2, const Piece& p3) {
				return (p1 == p2) && (p3 == p2);
			}

	};

	ostream& operator <<(std::ostream& os, const Piece& v);

	typedef std::size_t index_t;

	/**
	 * An 8x8 matrix of Piece objects.
	 * Left bottom square is (0,0), and right to is (7,7).
	 */
	class Board {
		public:
			/**
			 * Construct an instance that contains a Piece#EMPTY in each square.
			 */
			Board();
			/**
			 * What is on a square?
			 * @param colIndex
			 * @param rowIndex
			 * @return Piece::OUT_OF_BOUNDS if indexes are invalid
			 */
			const Piece& at(const index_t colIndex,const index_t rowIndex) const;
			const Piece& operator ()(const index_t colIndex,const index_t rowIndex) const {at(colIndex,rowIndex);}
			const Piece& operator ()(const Square& s) const {at(s.file(),s.rank());}
			/**
			 * Place a Piece on a square
			 * @param colIndex must be a valid index
			 * @param rowIndex must be a valid index
			 * @param value cannot be Piece::OUT_OF_BOUNDS
			 */
			void place(const std::size_t colIndex, const std::size_t rowIndex,const Piece &value);
			void operator()(const std::size_t colIndex, const std::size_t rowIndex,const Piece &value) {place(colIndex,rowIndex,value);}
			void operator()(const Square& s, const Piece &value) {place(s.file(),s.rank(),value);}
			void operator()(const Region& ss, const Piece &value); 
			int count(const Region& ss, const Piece &value) const;
			/** Max of value sequence length */
			int count_repeats(const Region& ss, const Piece &value) const;
			/** Return cend() if not found */
			Region::const_iterator find(const Region& ss, const Piece &value) const;
			bool operator< (const Board& o) const;
		private:
			std::array<Piece, 64> _data;
		public:
			typedef std::unique_ptr<Board> u_ptr;
	};

	ostream& operator <<(std::ostream& os, const Board& v);

	typedef std::forward_list<std::unique_ptr<Board>> BoardOwnerList;
	/**
	 * Where the player sits
	 */
	enum Side {
		South,
		North
	};

	/**
	 * A view of the board from a particular position,
	 * from a particular vantage point
	 */
	class BoardView {
		public:
			BoardView(const Board &brd, Side side = Side::South, std::size_t col = 0, std::size_t row = 0)
				: _vantageCol(col), _vantageRow(row), _vantageSide(side), _board(brd) {}
			void go(std::size_t col, std::size_t row) {
				_vantageCol = col;
				_vantageRow = row;
			};
			const Piece& anchor() const {return _board(_vantageCol, _vantageRow);};
			const index_t & col() const {return _vantageCol;};
			const index_t & row() const {return _vantageRow;};
			const Piece& relative(const index_t colOffset, const index_t rowOffset) const {
				const index_t delta_col = (_vantageSide == Side::South) ? colOffset : -colOffset;
				const index_t delta_row = (_vantageSide == Side::South) ? rowOffset : -rowOffset;
				return _board(_vantageCol + delta_col, _vantageRow + delta_row);
			}
		private:
			index_t _vantageCol, _vantageRow;
			Side _vantageSide;
			const Board &_board;
	};

	
}
