﻿#ifndef SDBM_HASH_H
#define SDBM_HASH_H


/*	Calculate hash for key
		note: this is the sdbm algorithm:
		hash(i) = hash(i-1) * 65599 + str[i];
	see: http://www.cse.yorku.ca/~oz/hash.html
	https://programmers.stackexchange.com/questions/49550
*/
inline unsigned int sdbm_hash( cstr key )
{
	unsigned int hash = 0;
	int c;
	while((c=*key++))
	{
		hash = (hash << 6) + (hash << 16) - hash + c;	// meistens eine Winzigkeit schneller als Multiplikation
	}

//	return hash;					// original algorithm
	return hash ^ (hash>>16);		// do some xor'ing to better support smaller required hash sizes
}



#endif // SDBM_HASH_H








