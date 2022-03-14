/**
 * @author Masaki Waga
 * @date 2022/03/13.
 */

#pragma once

#include "../include/forward_regional_elementary_language.hh"
#include "../include/backward_regional_elementary_language.hh"

/*!
 * @brief Fixture of the keys of the observation table in the paper
 */
struct SimpleObservationTableKeysFixture {
  learnta::ForwardRegionalElementaryLanguage p1, p2;
  learnta::BackwardRegionalElementaryLanguage s1, s2, s3;

  SimpleObservationTableKeysFixture() :
          p1(learnta::ForwardRegionalElementaryLanguage()),
          p2(p1.successor()),
          s1(learnta::BackwardRegionalElementaryLanguage()),
          s2(s1.predecessor('a')),
          s3(s2.predecessor()) {}
};