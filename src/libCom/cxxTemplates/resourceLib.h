/*
 *      $Id$
 *
 *      General hash table templates for fast indexing of resources
 *      of any base resource type and any resource identifier type. Fast 
 *      indexing is implemented with a hash lookup. The identifier type 
 *      implements the hash algorithm (or derives from one of the supplied 
 *      identifier types which provide a hashing routine). 
 *
 *      Unsigned integer and string identifier classes are supplied here.
 *
 *      Author  Jeffrey O. Hill 
 *              (string hash alg by Marty Kraimer and Peter K. Pearson)
 *
 *              johill@lanl.gov
 *              505 665 1831
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *      Copyright 1991, the Regents of the University of California,
 *      and the University of Chicago Board of Governors.
 *
 *      This software was produced under  U.S. Government contracts:
 *      (W-7405-ENG-36) at the Los Alamos National Laboratory,
 *      and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *      Initial development by:
 *              The Controls and Automation Group (AT-8)
 *              Ground Test Accelerator
 *              Accelerator Technology Division
 *              Los Alamos National Laboratory
 *
 *      Co-developed with
 *              The Controls and Computing Group
 *              Accelerator Systems Division
 *              Advanced Photon Source
 *              Argonne National Laboratory
 *
 *
 *  NOTES:
 *  .01 Storage for identifier must persist until an item is deleted
 *  .02 class T must derive from class ID and tsSLNode<T>
 *  
 */

#ifndef INCresourceLibh
#define INCresourceLibh 

#include <new>

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#ifndef assert // allow use of epicsAssert.h
#include <assert.h>
#endif

#include "tsSLList.h"
#include "shareLib.h"
#include "locationException.h"
typedef size_t resTableIndex;

template <class T, class ID> class resTableIter;

//
// class resTable <T, ID>
//
// This class stores resource entries of type T which can be efficiently 
// located with a hash key of type ID.
//
// NOTES: 
// 1)   class T _must_ derive from class ID and also from class tsSLNode<T>
//
// 2)   If the "resTable::show (unsigned level)" member function is called then 
//      class T must also implement a "show (unsigned level)" member function which
//      dumps increasing diagnostics information with increasing "level" to
//      standard out.
//
// 3)   Classes of type ID must implement the following member functions:
//
//          // equivalence test
//          bool operator == (const ID &);
//
//          // ID to hash index convert (see examples below)
//          resTableIndex hash (unsigned nBitsHashIndex) const; 
//
// 4)   Classes of type ID must provide the following member functions
//      (which will usually be static const inline for improved performance).
//      They determine the minimum and maximum number of elements in the hash 
//      table. If minIndexBitWidth() == maxIndexBitWidth() then the hash table
//      size is determined at compile time
//
//          inline static const unsigned maxIndexBitWidth ();
//          inline static const unsigned minIndexBitWidth ();
//
//          max number of hash table elements = 1 << maxIndexBitWidth();
//          min number of hash table elements = 1 << minIndexBitWidth();
//
// 5)   Storage for identifier of type ID must persist until the item of type 
//      T is deleted from the resTable
//
template <class T, class ID>
class resTable {
public:
    resTable ();
    virtual ~resTable();
    // Call " void T::show (unsigned level)" for each entry
    void show (unsigned level) const;
    void verify () const;
    int add (T &res); // returns -1 (id exists in table), 0 (success)
    T *remove (const ID &idIn); // remove entry
    T *lookup (const ID &idIn) const; // locate entry
    // Call (pT->*pCB) () for each entry
    void traverse ( void (T::*pCB)() );
    void traverseConst ( void (T::*pCB)() const ) const;
    unsigned numEntriesInstalled () const;
    //
    // exceptions thrown
    //
    class epicsShareClass dynamicMemoryAllocationFailed {};
    class epicsShareClass sizeExceedsMaxIndexWidth {};
private:
    tsSLList < T > *pTable;
    unsigned nextSplitIndex;
    unsigned hashIxMask;
    unsigned hashIxSplitMask;
    unsigned nInUse;
    resTableIndex hash ( const ID & idIn ) const;
    T *find ( tsSLList<T> &list, const ID &idIn ) const;
    T *findDelete ( tsSLList<T> &list, const ID &idIn );
    void splitBucket ();
    unsigned tableSize () const;
    resTable ( const resTable & );
    resTable & operator = ( const resTable & );
    friend class resTableIter<T,ID>;
};

//
// class resTableIter
//
// an iterator for the resource table class
//
template <class T, class ID>
class resTableIter {
public:
    resTableIter (const resTable<T,ID> &tableIn);
    T * next ();
    T * operator () ();
private:
    tsSLIter<T>             iter;
    unsigned                index;
    const resTable<T,ID>    &table;
};

//
// Some ID classes that work with the above template
//

//
// class intId
//
// signed or unsigned integer identifier (class T must be
// a signed or unsigned integer type)
//
// this class works as type ID in resTable <class T, class ID>
//
// 1<<MIN_INDEX_WIDTH specifies the minimum number of
// elements in the hash table within resTable <class T, class ID>.
// Set this parameter to zero if unsure of the correct minimum 
// hash table size.
//
// MAX_ID_WIDTH specifies the maximum number of ls bits in an 
// integer identifier which might be set at any time. 
//
// MIN_INDEX_WIDTH and MAX_ID_WIDTH are specified here at
// compile time so that the hash index can be produced 
// efficiently. Hash indexes are produced more efficiently 
// when (MAX_ID_WIDTH - MIN_INDEX_WIDTH) is minimized.
//
template <class T, unsigned MIN_INDEX_WIDTH=4u, 
    unsigned MAX_ID_WIDTH = sizeof(T)*CHAR_BIT>
class intId {
public:
    intId (const T &idIn);
    bool operator == (const intId &idIn) const;
    resTableIndex hash () const;
    const T getId() const;
    static const unsigned maxIndexBitWidth ();
    static const unsigned minIndexBitWidth ();
protected:
    T id;
};

//
// class chronIntIdResTable <ITEM>
//
// a specialized resTable which uses unsigned integer keys which are
// allocated in chronological sequence
// 
// NOTE: ITEM must public inherit from chronIntIdRes <ITEM>
//
class chronIntId : public intId<unsigned, 8u, sizeof(unsigned)*CHAR_BIT> 
{
public:
    chronIntId ( const unsigned &idIn );
};

template <class ITEM>
class chronIntIdResTable : public resTable<ITEM, chronIntId> {
public:
    chronIntIdResTable ();
    virtual ~chronIntIdResTable ();
    void add (ITEM &item);
private:
    unsigned allocId;
};

//
// class chronIntIdRes<ITEM>
//
// resource with unsigned chronological identifier
//
template <class ITEM>
class chronIntIdRes : public chronIntId, public tsSLNode<ITEM> {
    friend class chronIntIdResTable<ITEM>;
public:
    chronIntIdRes ();
private:
    void setId (unsigned newId);
};

//
// class stringId
//
// character string identifier
//
class epicsShareClass stringId {
public:
    enum allocationType {copyString, refString};
    stringId (const char * idIn, allocationType typeIn=copyString);
    virtual ~stringId();
    resTableIndex hash () const;
    bool operator == (const stringId &idIn) const;
    const char * resourceName() const; // return the pointer to the string
    void show (unsigned level) const;
    static const unsigned maxIndexBitWidth ();
    static const unsigned minIndexBitWidth ();
    //
    // exceptions
    //
    class epicsShareClass dynamicMemoryAllocationFailed {};
private:
    stringId & operator = ( const stringId & );
    stringId ( const stringId &);
    const char * pStr;
    const allocationType allocType;
    static const unsigned char fastHashPermutedIndexSpace[256];
};

/////////////////////////////////////////////////
// resTable<class T, class ID> member functions
/////////////////////////////////////////////////

// 
// resTable::resTable ()
//
template <class T, class ID>
resTable<T,ID>::resTable () :
    nextSplitIndex ( 0 ), 
    hashIxMask ( ( 1 << ID::minIndexBitWidth() ) - 1 ),
    hashIxSplitMask ( ( 1 << ( ID::minIndexBitWidth() + 1 ) ) - 1 ),
    nInUse ( 0 )
{
    unsigned newTableSize = this->hashIxSplitMask + 1;
    this->pTable = ( tsSLList<T> * ) 
        operator new ( newTableSize * sizeof ( tsSLList<T> ) );
    if ( ! this->pTable ) {
        throwWithLocation ( dynamicMemoryAllocationFailed () );
    }
    for ( unsigned i = 0u; i < newTableSize; i++ ) {
        new ( &this->pTable[i] ) tsSLList<T>;
    }
}

//
// resTable::remove ()
//
// remove a res from the resTable
//
template <class T, class ID>
inline T * resTable<T,ID>::remove ( const ID &idIn )
{
    tsSLList<T> &list = this->pTable[ this->hash(idIn) ];
    return this->findDelete ( list, idIn );
}

//
// resTable::lookup ()
//
// find an res in the resTable
//
template <class T, class ID>
inline T * resTable<T,ID>::lookup ( const ID &idIn ) const
{
    tsSLList<T> &list = this->pTable[this->hash(idIn)];
    return this->find (list, idIn);
}

//
// resTable::hash ()
//
template <class T, class ID>
inline resTableIndex resTable<T,ID>::hash ( const ID & idIn ) const
{
    resTableIndex h = idIn.hash ();
    resTableIndex h0 = h & this->hashIxMask;
    if ( h0 >= this->nextSplitIndex ) {
        return h0;
    }
    return h & this->hashIxSplitMask;
}

//
// resTable<T,ID>::show
//
template <class T, class ID>
void resTable<T,ID>::show ( unsigned level ) const
{
    unsigned N = this->tableSize ();
    printf ( "resTable with %u buckets and %u resources installed\n", 
        N, this->nInUse );

    if ( level >=1u ) {
        tsSLList <T> *pList = this->pTable;
        double X = 0.0;
        double XX = 0.0;
        unsigned maxEntries = 0u;
        for ( unsigned i = 0u; i < N; i++ ) {
            tsSLIter<T> pItem = pList[i].firstIter ();
            unsigned count = 0;
            while ( pItem.valid () ) {
                if ( level >= 3u ) {
                    pItem->show ( level );
                }
                count++;
                pItem++;
            }
            if ( count > 0u ) {
                X += count;
                XX += count * count;
                if ( count > maxEntries ) {
                    maxEntries = count;
                }
            }
        }
     
        double mean = X / N;
        double stdDev = sqrt( XX / N - mean * mean );
        printf ( 
    "entries per bucket: mean = %f std dev = %f max = %d\n",
            mean, stdDev, maxEntries );
        if ( X != this->nInUse ) {
            printf ("this->nInUse didnt match items counted which was %f????\n", X );
        }
    }
}

template <class T, class ID>
void resTable<T,ID>::verify () const
{
    unsigned N = this->tableSize ();
    unsigned total = 0u;
    tsSLList <T> *pList = this->pTable;
    for ( unsigned i = 0u; i < N; i++ ) {
        tsSLIter<T> pItem = pList[i].firstIter ();
        unsigned count = 0;
        while ( pItem.valid () ) {
            resTableIndex index = this->hash ( *pItem );
            assert ( index == i );
            count++;
            pItem++;
        }
        total += count;
    }
    assert ( total == this->nInUse );
}


//
// resTable<T,ID>::traverse
//
template <class T, class ID>
void resTable<T,ID>::traverse ( void (T::*pCB)() ) 
{
    tsSLList<T> *pList;

    pList = this->pTable;
    unsigned N = this->tableSize ();
    while ( pList < &this->pTable[N] ) {
        tsSLIter<T> pItem = pList->firstIter ();
        while ( pItem.valid () ) {
            tsSLIter<T> pNext = pItem;
            pNext++;
            ( pItem.pointer ()->*pCB ) ();
            pItem = pNext;
        }
        pList++;
    }
}

//
// resTable<T,ID>::traverseConst
//
template <class T, class ID>
void resTable<T,ID>::traverseConst ( void (T::*pCB)() const ) const
{
    const tsSLList<T> *pList;

    pList = this->pTable;
    unsigned N = this->tableSize ();
    while ( pList < &this->pTable[N] ) {
        tsSLIterConst<T> pItem = pList->firstIter ();
        while ( pItem.valid () ) {
            tsSLIterConst<T> pNext = pItem;
            pNext++;
            ( pItem.pointer ()->*pCB ) ();
            pItem = pNext;
        }
        pList++;
    }
}

template <class T, class ID>
inline unsigned resTable<T,ID>::numEntriesInstalled () const
{
    return this->nInUse;
}

template <class T, class ID>
unsigned resTable<T,ID>::tableSize () const
{
    return ( this->hashIxMask + 1 ) + this->nextSplitIndex;
}

template <class T, class ID>
void resTable<T,ID>::splitBucket ()
{
    // double the hash table when necessary
    // (this results in only a memcpy overhead, but 
    // no hashing or entry redistribution)
    if ( this->nextSplitIndex > this->hashIxMask ) {
        unsigned oldTableSize = this->hashIxSplitMask + 1;
        unsigned newTableSize = oldTableSize * 2;
        tsSLList<T> *pNewTable = ( tsSLList<T> * ) 
            operator new ( newTableSize * sizeof ( tsSLList<T> ), std::nothrow );
        if ( ! pNewTable ) {
            return;
        }
        unsigned oldTableOccupiedSize = ( this->hashIxMask + 1 ) + this->nextSplitIndex;
        // run the constructors using placement new
        unsigned i;
        for ( i = 0u; i < oldTableOccupiedSize; i++ ) {
            new ( &pNewTable[i] ) tsSLList<T> ( this->pTable[i] );
        }
        for ( i = oldTableOccupiedSize; i < newTableSize; i++ ) {
            new ( &pNewTable[i] ) tsSLList<T>;
        }
        // Run the destructors explicitly. Currently this destructor is a noop.
        // Currently the Tornado compiler cant deal with ~tsSLList<T>() but since 
        // its a NOOP we can find a workaround ...
        //
#       if ! defined (__GNUC__) || __GNUC__ > 2 || ( __GNUC__ == 2 && __GNUC_MINOR__ >= 72 )
            for ( i = 0; i < oldTableSize; i++ ) {
                this->pTable[i].~tsSLList<T>();
            }
#       endif
        operator delete ( this->pTable );
        this->pTable = pNewTable;
        this->hashIxMask = this->hashIxSplitMask;
        this->hashIxSplitMask = newTableSize - 1;
        this->nextSplitIndex = 0;
    }

    // rehash only the items in the split bucket
    tsSLList<T> tmp ( this->pTable[ this->nextSplitIndex ] );
    this->nextSplitIndex++;
    T *pItem = tmp.get();
    while ( pItem ) {
        resTableIndex index = this->hash(*pItem);
        tsSLList<T> &list = this->pTable[index];
        list.add ( *pItem );
        pItem = tmp.get();
    }
}

//
// add a res to the resTable
//
// (bad status on failure)
//
template <class T, class ID>
int resTable<T,ID>::add ( T &res )
{
    if ( this->nInUse > this->tableSize() ) {
        this->splitBucket ();
    }
    tsSLList<T> &list = this->pTable[this->hash(res)];
    if ( this->find ( list, res ) != 0 ) {
        return -1;
    }
    list.add ( res );
    this->nInUse++;
    return 0;
}

//
// find
// searches from where the iterator points to the
// end of the list for idIn
//
// iterator points to the item found upon return
// (or NULL if nothing matching was found)
//
template <class T, class ID>
T *resTable<T,ID>::find ( tsSLList<T> &list, const ID &idIn ) const
{
    tsSLIter <T> pItem = list.firstIter ();
    while ( pItem.valid () ) {
        const ID &id = *pItem;
        if ( id == idIn ) {
            break;
        }
        pItem++;
    }
    return pItem.pointer ();
}

//
// findDelete
// searches from where the iterator points to the
// end of the list for idIn
//
// iterator points to the item found upon return
// (or NULL if nothing matching was found)
//
// removes the item if it finds it
//
template <class T, class ID>
T *resTable<T,ID>::findDelete ( tsSLList<T> &list, const ID &idIn )
{
    tsSLIter <T> pItem = list.firstIter ();
    T *pPrev = 0;

    while ( pItem.valid () ) {
        const ID &id = *pItem;
        if ( id == idIn ) {
            if ( pPrev ) {
                list.remove ( *pPrev );
            }
            else {
                list.get ();
            }
            this->nInUse--;
            break;
        }
        pPrev = pItem.pointer ();
        pItem++;
    }
    return pItem.pointer ();
}

//
// ~resTable<T,ID>::resTable()
//
template <class T, class ID>
resTable<T,ID>::~resTable() 
{
    operator delete ( this->pTable );
}

//////////////////////////////////////////////
// resTableIter<T,ID> member functions
//////////////////////////////////////////////

//
// resTableIter<T,ID>::resTableIter ()
//
template <class T, class ID>
inline resTableIter<T,ID>::resTableIter (const resTable<T,ID> &tableIn) : 
    iter ( tableIn.pTable[0].firstIter () ), index (1), table ( tableIn ) {} 

//
// resTableIter<T,ID>::next ()
//
template <class T, class ID> 
T * resTableIter<T,ID>::next ()
{
    if ( this->iter.valid () ) {
        T *p = this->iter.pointer ();
        this->iter++;
        return p;
    }
    unsigned N = this->table.tableSize();
    while ( true ) {
        if ( this->index >= N ) {
            return 0;
        }
        this->iter = tsSLIter<T> ( this->table.pTable[this->index++].firstIter () );
        if ( this->iter.valid () ) {
            T *p = this->iter.pointer ();
            this->iter++;
            return p;
        }
    }
}

//
// resTableIter<T,ID>::operator () ()
//
template <class T, class ID>
inline T * resTableIter<T,ID>::operator () ()
{
    return this->next ();
}

//////////////////////////////////////////////
// chronIntIdResTable<ITEM> member functions
//////////////////////////////////////////////
inline chronIntId::chronIntId ( const unsigned &idIn ) : 
    intId<unsigned, 8u, sizeof(unsigned)*CHAR_BIT> ( idIn ) {}

//
// chronIntIdResTable<ITEM>::chronIntIdResTable()
//
template <class ITEM>
inline chronIntIdResTable<ITEM>::chronIntIdResTable () : 
    resTable<ITEM, chronIntId> (), allocId(1u) {}

//
// chronIntIdResTable<ITEM>::~chronIntIdResTable()
// (not inline because it is virtual)
//
template <class ITEM>
chronIntIdResTable<ITEM>::~chronIntIdResTable() {}

//
// chronIntIdResTable<ITEM>::add()
//
// NOTE: This detects (and avoids) the case where 
// the PV id wraps around and we attempt to have two 
// resources with the same id.
//
template <class ITEM>
inline void chronIntIdResTable<ITEM>::add (ITEM &item)
{
    int status;
    do {
        item.chronIntIdRes<ITEM>::setId (allocId++);
        status = this->resTable<ITEM,chronIntId>::add (item);
    }
    while (status);
}

/////////////////////////////////////////////////
// chronIntIdRes<ITEM> member functions
/////////////////////////////////////////////////

//
// chronIntIdRes<ITEM>::chronIntIdRes
//
template <class ITEM>
inline chronIntIdRes<ITEM>::chronIntIdRes () : chronIntId (UINT_MAX) {}

//
// id<ITEM>::setId ()
//
// workaround for bug in DEC compiler
//
template <class ITEM>
inline void chronIntIdRes<ITEM>::setId (unsigned newId) 
{
    this->id = newId;
}

/////////////////////////////////////////////////
// intId member functions
/////////////////////////////////////////////////

//
// intId::intId ()
//
template <class T, unsigned MIN_INDEX_WIDTH, unsigned MAX_ID_WIDTH>
intId<T, MIN_INDEX_WIDTH, MAX_ID_WIDTH>::intId (const T &idIn) 
    : id (idIn) {}

//
// intId::operator == ()
//
template <class T, unsigned MIN_INDEX_WIDTH, unsigned MAX_ID_WIDTH>
inline bool intId<T, MIN_INDEX_WIDTH, MAX_ID_WIDTH>::operator == 
        (const intId<T, MIN_INDEX_WIDTH, MAX_ID_WIDTH> &idIn) const
{
    return this->id == idIn.id;
}

//
// intId::getId ()
//
template <class T, unsigned MIN_INDEX_WIDTH, unsigned MAX_ID_WIDTH>
inline const T intId<T, MIN_INDEX_WIDTH, MAX_ID_WIDTH>::getId () const
{
    return this->id;
}

//
// const unsigned intId::minIndexBitWidth ()
//
template <class T, unsigned MIN_INDEX_WIDTH, unsigned MAX_ID_WIDTH>
inline const unsigned intId<T, MIN_INDEX_WIDTH, MAX_ID_WIDTH>::minIndexBitWidth ()
{
    return MIN_INDEX_WIDTH;
}


//
//  const unsigned intId::maxIndexBitWidth ()
//
template <class T, unsigned MIN_INDEX_WIDTH, unsigned MAX_ID_WIDTH>
inline const unsigned intId<T, MIN_INDEX_WIDTH, MAX_ID_WIDTH>::maxIndexBitWidth ()
{
    return sizeof (resTableIndex) * CHAR_BIT;
}

//
// integerHash()
//
// converts any integer into a hash table index
//
template < class T >
inline resTableIndex integerHash ( unsigned MIN_INDEX_WIDTH,
                                  unsigned MAX_ID_WIDTH, const T &id )
{
    resTableIndex hashid = static_cast <resTableIndex> ( id );

    //
    // the intent here is to gurantee that all components of the 
    // integer contribute even if the resTableIndex returned might
    // index a small table.
    //
    // On most compilers the optimizer will unroll this loop so this
    // is actually a very small inline function
    //
    // Experiments using the microsoft compiler show that this isnt 
    // slower than switching on the architecture size and unrolling the
    // loop explicitly (that solution has resulted in portability
    // problems in the past).
    //
    unsigned width = MAX_ID_WIDTH;
    do {
        width >>= 1u;
        hashid ^= hashid>>width;
    } while (width>MIN_INDEX_WIDTH);

    //
    // the result here is always masked to the
    // proper size after it is returned to the "resTable" class
    //
    return hashid;
}


//
// intId::hash()
//
template <class T, unsigned MIN_INDEX_WIDTH, unsigned MAX_ID_WIDTH>
inline resTableIndex intId<T, MIN_INDEX_WIDTH, MAX_ID_WIDTH>::hash () const
{
    return integerHash ( MIN_INDEX_WIDTH, MAX_ID_WIDTH, this->id );
}

////////////////////////////////////////////////////
// stringId member functions
////////////////////////////////////////////////////

//
// stringId::operator == ()
//
inline bool stringId::operator == 
        (const stringId &idIn) const
{
    if (this->pStr!=NULL && idIn.pStr!=NULL) {
        return strcmp(this->pStr,idIn.pStr)==0;
    }
    else {
        return false; // not equal
    }
}

//
// stringId::resourceName ()
//
inline const char * stringId::resourceName () const
{
    return this->pStr;
}

static const unsigned stringIdMinIndexWidth = CHAR_BIT;
static const unsigned stringIdMaxIndexWidth = sizeof ( unsigned );

//
// const unsigned stringId::minIndexBitWidth ()
//
inline const unsigned stringId::minIndexBitWidth ()
{
    return stringIdMinIndexWidth;
}

//
// const unsigned stringId::maxIndexBitWidth ()
//
inline const unsigned stringId::maxIndexBitWidth ()
{
    return stringIdMaxIndexWidth;
}

#ifdef instantiateRecourceLib

//
// stringId::stringId()
//
stringId::stringId (const char * idIn, allocationType typeIn) :
    allocType (typeIn)
{
    if (typeIn==copyString) {
        unsigned nChars = strlen (idIn) + 1u;

        this->pStr = new char [nChars];
        if (this->pStr!=0) {
            memcpy ((void *)this->pStr, idIn, nChars);
        }
        else {
            throwWithLocation ( dynamicMemoryAllocationFailed () );
        }
    }
    else {
        this->pStr = idIn;
    }
}

//
// stringId::show ()
//
void stringId::show (unsigned level) const
{
    if (level>2u) {
        printf ("resource id = %s\n", this->pStr);
    }
}

//
// stringId::~stringId()
//
//
// this needs to be instantiated only once (normally in libCom)
//
stringId::~stringId()
{
    if (this->allocType==copyString) {
        if (this->pStr!=NULL) {
            //
            // the microsoft and solaris compilers will 
            // not allow a pointer to "const char"
            // to be deleted 
            //
            // the HP-UX compiler gives us a warning on
            // each cast away of const, but in this case
            // it cant be avoided. 
            //
            // The DEC compiler complains that const isnt 
            // really significant in a cast if it is present.
            //
            // I hope that deleting a pointer to "char"
            // is the same as deleting a pointer to 
            // "const char" on all compilers
            //
            delete [] const_cast<char *>(this->pStr);
        }
    }
}

//
// stringId::hash()
//
// This is a modification of the algorithm described in 
// "Fast Hashing of Variable Length Text Strings", Peter K. Pearson, 
// Communications of the ACM, June 1990. The initial modifications 
// were designed by Marty Kraimer. Some additional minor optimizations
// by Jeff Hill.
//
resTableIndex stringId::hash() const
{
    const unsigned char *pUStr = 
        reinterpret_cast < const unsigned char * > ( this->pStr );

    if ( ! pUStr ) {
        return 0u;
    }

    unsigned h0 = 0;
    unsigned h1 = 0;
    unsigned h2 = 0;
    unsigned h3 = 0;
    unsigned c;

    while ( true ) {

        c = * ( pUStr++ );
        if ( c == 0 ) {
            break;
        }
        h0 = fastHashPermutedIndexSpace [ h0 ^ c ];

        c = * ( pUStr++ );
        if ( c == 0 ) {
            break;
        }
        h1 = fastHashPermutedIndexSpace [ h1 ^ c ];

        c = * ( pUStr++ );
        if ( c == 0 ) {
            break;
        }
        h2 = fastHashPermutedIndexSpace [ h2 ^ c ];

        c = * ( pUStr++ );
        if ( c == 0 ) {
            break;
        }
        h3 = fastHashPermutedIndexSpace [ h3 ^ c ];
    }

    h0 = ( h3 << 24 ) | ( h2 << 16 ) | ( h1 << 8 ) | h0;

    return integerHash ( stringIdMinIndexWidth, stringIdMaxIndexWidth, h0 );
}

//
// This is a modification of the algorithm described in
// "Fast Hashing of Variable Length Text Strings", Peter K. 
// Pearson, Communications of the ACM, June 1990
// The modifications were designed by Marty Kraimer
//
const unsigned char stringId::fastHashPermutedIndexSpace[256] = {
 39,159,180,252, 71,  6, 13,164,232, 35,226,155, 98,120,154, 69,
157, 24,137, 29,147, 78,121, 85,112,  8,248,130, 55,117,190,160,
176,131,228, 64,211,106, 38, 27,140, 30, 88,210,227,104, 84, 77,
 75,107,169,138,195,184, 70, 90, 61,166,  7,244,165,108,219, 51,
  9,139,209, 40, 31,202, 58,179,116, 33,207,146, 76, 60,242,124,
254,197, 80,167,153,145,129,233,132, 48,246, 86,156,177, 36,187,
 45,  1, 96, 18, 19, 62,185,234, 99, 16,218, 95,128,224,123,253,
 42,109,  4,247, 72,  5,151,136,  0,152,148,127,204,133, 17, 14,
182,217, 54,199,119,174, 82, 57,215, 41,114,208,206,110,239, 23,
189, 15,  3, 22,188, 79,113,172, 28,  2,222, 21,251,225,237,105,
102, 32, 56,181,126, 83,230, 53,158, 52, 59,213,118,100, 67,142,
220,170,144,115,205, 26,125,168,249, 66,175, 97,255, 92,229, 91,
214,236,178,243, 46, 44,201,250,135,186,150,221,163,216,162, 43,
 11,101, 34, 37,194, 25, 50, 12, 87,198,173,240,193,171,143,231,
111,141,191,103, 74,245,223, 20,161,235,122, 63, 89,149, 73,238,
134, 68, 93,183,241, 81,196, 49,192, 65,212, 94,203, 10,200, 47
};

#endif // if instantiateRecourceLib is defined

#endif // INCresourceLibh

