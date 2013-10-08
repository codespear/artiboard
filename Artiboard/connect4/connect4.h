#pragma once
#include <game.h>
using namespace arti;

int ply_of(const Board&b);

class Connect4: public GameSpecification {
	void setup(arti::Board&) const override;
	MatchOutcome outcome_of(const Position&) const override;
	void collectMoves(const Position& pos, Move::SharedFWList &result) const override;
};