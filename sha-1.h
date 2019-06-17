/***************************************************************************
****************************************************************************
* Filename        : sha-1.cpp
* Author          : Jishnu Murali Thampan
* Description     : Interface of SHA-1 Alogrithm
* 		    Note: This version does not support message sizes > 512 bytes
****************************************************************************/

#ifndef SHA1_HPP
#define SHA1_HPP

typedef unsigned char           uint8_t;	/**< \brief Represents an empty string */
typedef unsigned long long      uint64_t;   /**< \brief Represents an empty string */
typedef unsigned int            uint32_t;   /**< \brief Represents an empty string */

void generate_sha1_hash();

#endif /* SHA1_HPP */
