#include "game.h"
#include "systemex.h"

#define FOR_SQUARES(row,col) for (index_t row = 0; row < 8; row++) for (index_t col = 0; col < 8; col++)

namespace arti {
		void PlayLine::add(unique_ptr<Board> &brd) {
		_plies.push_back(shared_ptr<Position>(new Position(last().ply().next(), brd)));
	}

	std::unique_ptr<Board> Move::apply_to(const Board &brd) const {
		Board::u_ptr result(new Board(brd));
		FOR_EACH(step,_steps) {
			(*step)->apply_on(*result);
		}
		return result;
	}


	std::unique_ptr<Board> GameSpecification::initialBoard() const {
		auto result = new Board();
		setup(*result);
		return unique_ptr<Board>(result);
	}

	int GameSpecification::collectBoards(const Position& pos, Board::u_ptr_list &result) const {
		Move::SharedFWList moves;
		collectMoves(pos, moves);
		if (moves.empty())
			return 0;
		else {
			int c = 0;
			for(auto &m : moves) {
				result.push_front(m->apply_to(pos.board()));
				c++;
			}
			return c;
		}
	}



	void GameSpecificationWithLocalSteps::collectMoves(const Position& pos, Move::SharedFWList &result) const {
		BoardView view(pos.board(),pos.ply().side_to_move());
		Step::SharedFWList steps;
		int stepIndex = 0;
		FOR_SQUARES(r,c) {
			stepIndex = 0;
			view.go(r,c);
			steps.clear();
			collectSteps(pos,view,stepIndex,steps);
			for(auto &step : steps) {
				if ((step)->outcome() == StepOutcome::EndsMoveAndContinue)
					throw std::runtime_error("open steps not implemented yet");
				result.emplace_front(new Move(step));
			}
		}
	}

	ostream& operator <<(std::ostream& os, const Board& v) {
		for (int r = 7; r >= 0; r--) {
			os << r << " ";
			for (index_t c = 0; c < 8; c++) {
				os << v(c,r) << " ";
			}
			os << std::endl;
		}
		os << "  ";
		for (index_t c = 0; c < 8; c++) 
			os << c << " ";
		os << std::endl;
		return os;
	}

	const Ply Ply::ZERO(0);

	PlayLine::PlayLine(unique_ptr<Board> initial) {
		_plies.emplace_back(new Position(Ply::ZERO, initial));
	};

	ostream& operator <<(ostream& os, const PlayLine& v) {
		for(auto &p:v.sequence()) {
			os << "Ply: " << p->ply().index() << std::endl << p->board() << std::endl;
		}
		return os;
	}

	Match::Match(const GameSpecification& spec,  MoveChooser& chooser):
			_spec(spec),
			_chooser(chooser),
			_line(spec.initialBoard()),
			_outcome(MatchOutcome::Unknown) {
	}

	void Match::play() {
		if (!_line.last().is_root())
			throw std::runtime_error("cannot play again");
		Move::SharedFWList moves;
		while (_outcome == MatchOutcome::Unknown) {
			auto &pos = _line.last();
			std::forward_list<unique_ptr<Board>> boards;
			const auto count = _spec.collectBoards(pos, boards);
			if (count == 0)
				_outcome = MatchOutcome::Draw;
			else {
				ASSERT(boards.begin()!=boards.end());
				auto selected = count==1?boards.begin():_chooser.select(pos, boards);
				ENSURE(selected != boards.end(), "select returned end");
				_line.add(*selected);
				_outcome = _spec.outcome_of(_line.last());
			}
		}
	}

	ostream& operator <<(std::ostream& os, const Match& v) {
		os << "Match had " << v.line().sequence().size() << " moves" << std::endl;
		os << v.line();
		return os;
	}

 ostream& operator <<(std::ostream& os, const MatchOutcome& v) {
		static const char* chars = "usnd";
		os << chars[v];
		return os;
	}


}
