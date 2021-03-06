	tree grammar FeatTree;

	options{ 
	  language = C;
		tokenVocab = Feat;
		ASTLabelType = pANTLR3_BASE_TREE;
	}

 	@includes {
		#include <map>
		#include <exception>
		#include <stdlib.h>
		#include <iostream>
		#include "../board.h"
		#include "../log.h"
		#include "../feat_program.h"
		using namespace arti;
	}

	@members {
		std::map<pFeatTree,FeatureProgram *> programs;

		FeatureProgram * pgm(pFeatTree ctx) {
			auto result = programs.find(ctx);
			if (result == programs.end()) {
				auto newP = new FeatureProgram();
				programs[ctx] = newP;
				return newP;
			} else
				return result->second;
		}
		
		#define _ctx pgm(CTX)
	}
 
	program returns [FeatureProgram * value]
		:	clause+ {
				auto result = _ctx;
				programs.erase(CTX);
				return result;
			}
		;

	clause
		: ^('region' n=ID r=region) {
				string name = (const char*)$n.text->chars;
				_ctx->regions().add(name,$r.value);
			} 
		| ^('stateset' n=ID v=state_set) {
				string name = (const char*)$n.text->chars;
				_ctx->states().add(name,$v.value);
			}
		| ^('formula' n=ID e=expression) {
				string name = (const char*)$n.text->chars;
				_ctx->formulas().add(name,$e.value);			
		}
		| ^('function' n=ID t=term_list) {
				string name = (const char*)$n.text->chars;
				_ctx->functions().add(name, $t.value);		
		}	
		;

	term_list returns [FeatureFunction * value]
		: t=fun_term {$value = new FeatureFunction();$value->terms().emplace_back(upFeatureTerm($t.value));}
		  (t=fun_term {$value->terms().emplace_back(upFeatureTerm($t.value));})*
      
		;
	fun_term returns [FeatureTerm * value]
		: ^('*' f=FLOAT n=ID) {
				string name = (const char*)$n.text->chars;
				auto w = (const char*)$f.text->chars;
				_ctx->formulas().at(name); // make sure the name exists
				$value = new FeatureTermWithFormula(std::atof(w),name);		
		}
		| ^('*' f=FLOAT e=expression)	{
				auto w = (const char*)$f.text->chars;
				$value = new FeatureTermWithExpression(std::atof(w),$e.value);		
		}
		;

	expression returns [FeatureExpression * value]
		: ^('@' s=state_set_ref r=region_ref) {$value = new GroundExpression($s.value,$r.value);}
		| ^('!' e=expression) {$value = new NotExpression($e.value);}
		| ^('|' e1=expression e2=expression) {$value = new OrExpression($e1.value,$e2.value);}
		| ^('&' e1=expression e2=expression) {$value = new AndExpression($e1.value,$e2.value);}
		;

	state_set_ref returns [string value]
	 : s = state_set {
	 			$value = _ctx->states().assign_name(s);
	 		}
	 | n=ID {
		 		$value = (const char*)$n.text->chars;
	 				if (!_ctx->states().has_name($value))
	 			throw runtime_error("State set with name '" + $value + "' has not been defined");
 			}
 		;

	state_set returns [StateSet value]
	 : ^('{' (s=ID {
	 		 $value.insert((const char*)$s.text->chars);
	 	  })+)	
	 ;

	region_ref returns [string value]
	 : s = region {
	 			$value = _ctx->regions().assign_name(s);
	 		}
	 | n=ID {
	 		$value = (const char*)$n.text->chars;
	 		if (!_ctx->regions().has_name($value))
	 			throw runtime_error("Region with name '" + $value + "' has not been defined");
	 }
	 ;

	region returns [Region value]
		: ^('{' (s=square{$value.insert($s.value);})+)
		;

	square returns [Square value]
		: ^(',' c=INTEGER r=INTEGER) {
			$value = Square(
				(index_t) atoi((const char*)$c.text->chars),
				(index_t) atoi((const char*)$r.text->chars));}
		;	