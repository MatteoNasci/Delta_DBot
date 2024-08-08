#pragma once
#ifndef H_MLN_DB_DB_AUTHORIZER_CODE_H
#define H_MLN_DB_DB_AUTHORIZER_CODE_H

namespace mln {			
	enum class db_authorizer_code {
		/*******************************3rd**************4th************/
		create_index =         1,   /* Index Name      Table Name      */
		create_table =         2,   /* Table Name      NULL            */
		create_temp_index =    3,   /* Index Name      Table Name      */
		create_temp_table =    4,   /* Table Name      NULL            */
		create_temp_trigger =  5,   /* Trigger Name    Table Name      */
		create_temp_view =     6,   /* View Name       NULL            */
		create_trigger =       7,   /* Trigger Name    Table Name      */
		create_view =          8,   /* View Name       NULL            */
		del =                  9,   /* Table Name      NULL            */
		drop_index =          10,   /* Index Name      Table Name      */
		drop_table =          11,   /* Table Name      NULL            */
		drop_temp_index =     12,   /* Index Name      Table Name      */
		drop_temp_table =     13,   /* Table Name      NULL            */
		drop_temp_trigger =   14,   /* Trigger Name    Table Name      */
		drop_temp_view =      15,   /* View Name       NULL            */
		drop_trigger =        16,   /* Trigger Name    Table Name      */
		drop_view =           17,   /* View Name       NULL            */
		insert =              18,   /* Table Name      NULL            */
		pragma =              19,   /* Pragma Name     1st arg or NULL */
		read =                20,   /* Table Name      Column Name     */
		select =              21,   /* NULL            NULL            */
		transaction =         22,   /* Operation       NULL            */
		update =              23,   /* Table Name      Column Name     */
		attach =              24,   /* Filename        NULL            */
		detach =              25,   /* Database Name   NULL            */
		alter_table =         26,   /* Database Name   Table Name      */
		re_index =            27,   /* Index Name      NULL            */
		analyze =             28,   /* Table Name      NULL            */
		create_vtable =       29,   /* Table Name      Module Name     */
		drop_vtable =         30,   /* Table Name      Module Name     */
		function =            31,   /* NULL            Function Name   */
		savepoint =           32,   /* Operation       Savepoint Name  */
		copy =                 0,   /* No longer used */
		recursive =           33,   /* NULL            NULL            */
	};
}

#endif //H_MLN_DB_DB_AUTHORIZER_CODE_H