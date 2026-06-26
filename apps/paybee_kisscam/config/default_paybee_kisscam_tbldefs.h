#ifndef paybee_kisscam_TBLDEFS_H
#define paybee_kisscam_TBLDEFS_H

#include "common_types.h"
#include "paybee_kisscam_tbl.h"


typedef struct {

    /**
     * `0` : Image Download Not-Started
     * `1` : Image Download On-going
     * `2` : Image Download Done
     */
    uint8_t MemoryState;

    /**
     * Last Image number of specific memory slot. Used for image naming.
     * e.g.) If 10 image downloaded on specific memory slot, then `LastImgIdx` become `10`
     */
    uint8_t LastImgIdx;

    /**
     * `480` bits
     * Each bit indicates the download state of corresponded line
     * e.g) If line 10 downloaded, Idx [1]'s 3rd bit becomes `1`
     */
    uint8_t LineState[60];

} paybee_kisscam_MemoryEntry_t;


#endif /* paybee_kisscam_TBLDEFS_H */