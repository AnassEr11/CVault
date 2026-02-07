#ifndef DB_INII_SERVICE_H
#define DB_INII_SERVICE_H

#include <stdbool.h>

/**
 * @brief Initializes the database schema.
 *
 * @return true if successful, false otherwise.
 */
bool init_schema();

/**
 * @brief Checks if the database is initialized and has a valid schema.
 *
 * @return true if the schema is valid and ready for use.
 */
bool is_init_schema();

#endif // !DB_INII_SERVICE_H
