//
// Entry for facility coverage heap
//

#ifndef FCLA_FACILITYRANK_H
#define FCLA_FACILITYRANK_H

struct FacilityRank {
public:
    long coverage;
    long lastUsed;
    FacilityRank() {}
    FacilityRank(long c, long u) {
        coverage = c;
        lastUsed = u;
    }
    ~FacilityRank() {}

    inline bool operator < (const FacilityRank& rhs) const {
        if (coverage == rhs.coverage) {
            return lastUsed > rhs.lastUsed; //more recently used is worse
        }
        return coverage < rhs.coverage;
    }

    inline bool operator > (const FacilityRank& rhs) const {
        if (coverage == rhs.coverage) {
            return lastUsed < rhs.lastUsed; //more recently used is worse
        }
        return coverage > rhs.coverage;
    }
};

#endif //FCLA_FACILITYRANK_H
