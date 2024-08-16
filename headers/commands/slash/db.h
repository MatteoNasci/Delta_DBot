#pragma once
#ifndef H_MLN_DB_SHOW_H
#define H_MLN_DB_SHOW_H

#include "commands/slash/base_slashcommand.h"

namespace mln {
    class db final : public base_slashcommand {
    private:
        size_t saved_insert_stmt, saved_insert_replace_stmt, saved_select_stmt, saved_select_verbose_stmt, saved_show_records_stmt, saved_show_records_verbose_stmt, saved_update_stmt, saved_update_desc_stmt, saved_remove_stmt;
        int saved_insert_guild, saved_insert_user, saved_insert_file_name, saved_insert_file_url, saved_insert_desc;
        int saved_insert_replace_guild, saved_insert_replace_user, saved_insert_replace_file_name, saved_insert_replace_file_url, saved_insert_replace_desc;
        int saved_select_guild, saved_select_file_name;
        int saved_select_verbose_guild, saved_select_verbose_file_name;
        int saved_update_guild, saved_update_user, saved_update_file_name, saved_update_file_url, saved_update_desc;
        int saved_update_desc_guild, saved_update_desc_user, saved_update_desc_file_name, saved_update_desc_desc;
        int saved_remove_guild, saved_remove_user, saved_remove_file_name;
        bool valid_stmt;
    public:
        db(bot_delta* const delta);
        dpp::task<void> command(const dpp::slashcommand_t& event_data) override;
    };
}

#endif //H_MLN_DB_SHOW_H