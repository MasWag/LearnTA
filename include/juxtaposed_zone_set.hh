/**
 * @author Masaki Waga
 * @date 2022/03/13.
 */

#pragma once

#include <ostream>
#include "juxtaposed_zone.hh"
#include "timed_condition_set.hh"

namespace learnta {
  class JuxtaposedZoneSet {
  private:
    std::vector<JuxtaposedZone> zones;
  public:
    JuxtaposedZoneSet(const TimedConditionSet &left, const TimedCondition &right) {
      zones.resize(left.size());
      std::transform(left.getConditions().begin(), left.getConditions().end(), zones.begin(),
                     [&right](const TimedCondition &condition) {
                       return condition ^ right;
                     });
    }

    /*!
     * @brief Juxtapose with variable renaming
     *
     * @sa JuxtaposedZone::JuxtaposedZone
     * @post All the resulting JuxtaposedZone is canonical
     */
    JuxtaposedZoneSet(const TimedConditionSet &left, const TimedCondition &right,
                      const std::size_t commonVariableSize) {
      zones.resize(left.size());
      std::transform(left.getConditions().begin(), left.getConditions().end(), zones.begin(),
                     [&](const TimedCondition &condition) {
                       return condition.juxtaposeRight(right, static_cast<Eigen::Index>(commonVariableSize));
                     });
    }

    /*!
     * @brief Juxtapose with variable renaming
     *
     * @sa JuxtaposedZone::JuxtaposedZone
     * @post All the resulting JuxtaposedZone is canonical
     */
    JuxtaposedZoneSet(const TimedCondition &left, const TimedConditionSet &right,
                      const std::size_t commonVariableSize) {
      zones.resize(right.size());
      std::transform(right.getConditions().begin(), right.getConditions().end(), zones.begin(),
                     [&](const TimedCondition &condition) {
                       return condition.juxtaposeLeft(left, static_cast<Eigen::Index>(commonVariableSize));
                     });
    }

    /*!
     * @brief Add renaming constraints
     */
    void addRenaming(const std::vector<std::pair<std::size_t, std::size_t>> &renaming) {
      for (auto it = this->zones.begin(); it != this->zones.end();) {
        it->addRenaming(renaming);
        if (it->isSatisfiableNoCanonize()) {
          it++;
        } else {
          it = this->zones.erase(it);
        }
      }
    }

    /*!
     * @brief Check the equivalence of two juxtaposed zone sets
     *
     * @pre Both of the juxtaposed zones are canonical
     */
    bool operator==(const JuxtaposedZoneSet &another) const {
      return this->zones.size() == another.zones.size() &&
             std::all_of(this->zones.begin(), this->zones.end(), [&](const JuxtaposedZone &zone) {
               return std::any_of(another.zones.begin(), another.zones.end(), [&](const JuxtaposedZone &anotherZone) {
                 return zone.strictEqual(anotherZone);
               });
             });
    }

    bool operator!=(const JuxtaposedZoneSet &another) const {
      return !(*this == another);
    }

    friend std::ostream &operator<<(std::ostream &os, const JuxtaposedZoneSet &set) {
      bool isFirst = true;
      for (const JuxtaposedZone &zone: set.zones) {
        if (!isFirst) {
          os << ", ";
        }
        os << zone;
        isFirst = false;
      }

      return os;
    }
  };
}