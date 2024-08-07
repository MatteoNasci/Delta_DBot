#pragma once
#ifndef H_MLN_DB_DB_CHECKPOINT_MODE_H
#define H_MLN_DB_DB_CHECKPOINT_MODE_H

namespace mln {
	enum class db_checkpoint_mode {
		checkpoint_passive = 0,  /* Do as much as possible w/o blocking */
		checkpoint_full = 1,  /* Wait for writers, then checkpoint */
		checkpoint_restart = 2,  /* Like FULL but wait for readers */
		checkpoint_truncate = 3,  /* Like RESTART but also truncate WAL */
	};
}

#endif //H_MLN_DB_DB_CHECKPOINT_MODE_H