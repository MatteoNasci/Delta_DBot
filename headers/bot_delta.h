#pragma once
#ifndef H_MLN_DB_BOT_DELTA_H
#define H_MLN_DB_BOT_DELTA_H

#include "general/events.h"
#include <dpp/cluster.h>
#include <dpp/snowflake.h>
#include <string>
#include <memory>
#include "bot_delta_data.h"


class cmd_runner;
class cmd_ctx_runner;
class form_submit_runner;
class select_click_runner;

class bot_delta{
    public:
        bot_delta_data_t data;
    public:
        bot_delta(const bool register_cmds);
        bot_delta() = delete;
        /**
	     * @brief bot_delta is non-copyable
	     */
	    bot_delta(const bot_delta&) = delete;

	    /**
	     * @brief bot_delta is non-moveable
	     */
	    bot_delta(const bot_delta&&) = delete;

	    /**
	     * @brief bot_delta is non-copyable
	     */
	    bot_delta& operator=(const bot_delta&) = delete;

	    /**
	     * @brief bot_delta is non-moveable
	     */
	    bot_delta& operator=(const bot_delta&&) = delete;
        std::string start();
    private:   
               
        cmd_runner cmds;
        cmd_ctx_runner ctxs;
        form_submit_runner forms;
        select_click_runner selects;
    private:
        void init(const bool register_cmds);
};

#endif