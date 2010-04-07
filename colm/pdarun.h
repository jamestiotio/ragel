/*
 *  Copyright 2007-2009 Adrian Thurston <thurston@complang.org>
 */

/*  This file is part of Colm.
 *
 *  Colm is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  Colm is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with Colm; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#ifndef _PDARUN_H
#define _PDARUN_H

#include <iostream>
#include "bytecode.h"
#include "rtvector.h"

using std::ostream;

struct Tree;
struct ParseData;
struct FsmRun;
struct KlangEl;
struct PdaTables;
struct FsmTables;
struct InputStream;
struct InputStreamAccum;

struct Kid
{
	/* The tree needs to be first since pointers to kids are used to reference
	 * trees on the stack. A pointer to the word that is a Tree* is cast to
	 * a Kid*. */
	Tree *tree;
	Kid *next;
};

struct Ref
{
	Kid *kid;
	Ref *next;
};

struct Tree
{
	/* First four will be overlaid in other structures. */
	short id;
	unsigned short flags;
	long refs;
	Kid *child;

	Head *tokdata;
};

struct ParseTree
{
	/* Entire structure must overlay Tree. */
	short id;
	unsigned short flags;
	long refs;
	Kid *child;

	Head *tokdata;

	/* Parsing algorithm. */
	long state;
	long region;
	char causeReduce;
	char retry_lower;
	char retry_upper;
};

struct Int
{
	/* Must overlay Tree. */
	short id;
	unsigned short flags;
	long refs;
	Kid *child;

	long value;
};

struct Pointer
{
	/* Must overlay Tree. */
	short id;
	unsigned short flags;
	long refs;
	Kid *child;

	Kid *value;
};

struct Str
{
	/* Must overlay Tree. */
	short id;
	unsigned short flags;
	long refs;
	Kid *child;

	Head *value;
};

struct ListEl
{
	/* Must overlay kid. */
	Tree *value;
	ListEl *next;
	ListEl *prev;

	ListEl() { }
	ListEl( Tree *value )
		: value(value) { }
};

struct List
{
	/* Must overlay Tree. */
	short id;
	unsigned short flags;
	long refs;
	ListEl *head;

	ListEl *tail;
	long listLen;
	GenericInfo *genericInfo;

	void prepend(ListEl *new_el) { addBefore(head, new_el); }
	void append(ListEl *new_el)  { addAfter(tail, new_el); }

	void addAfter( ListEl *prev_el, ListEl *new_el );
	void addBefore( ListEl *next_el, ListEl *new_el );

	ListEl *detachFirst()        { return detach(head); }
	ListEl *detachLast()         { return detach(tail); }
	ListEl *detach(ListEl *el);

	long length() const 
		{ return listLen; }
};

struct MapEl
{
	/* Must overlay Kid. */
	Tree *tree;
	MapEl *next;
	MapEl *prev;

	MapEl *left, *right, *parent;
	long height;
	Tree *key;
};

struct Map
{
	/* Must overlay Tree. */
	short id;
	unsigned short flags;
	long refs;
	MapEl *head;

	MapEl *tail;
	MapEl *root;
	long treeSize;
	GenericInfo *genericInfo;
};

void mapListAbandon( Map *map );
void mapEmpty( Map *map );
MapEl *mapListDetach( MapEl *el );
MapEl *mapCopyBranch( Program *p, Map *map, MapEl *el, Kid *oldNextDown, Kid *&newNextDown );
MapEl *mapInsert( Program *prg, Map *map, MapEl *element, MapEl **lastFound = 0 );
MapEl *mapInsert( Program *prg, Map *map, Tree *key, MapEl **lastFound = 0 );
MapEl *mapImplFind( Program *prg, Map *map, Tree *key );
MapEl *mapDetach( Program *prg, Map *map, Tree *key );
MapEl *mapDetach( Program *prg, Map *map, MapEl *element );
void mapImplRemove( Program *prg, Map *map, MapEl *element );
bool mapImplRemove( Program *prg, Map *map, Tree *key );
MapEl *mapFindFirstUnbalEl( Map *map, MapEl *element );
MapEl *mapFindFirstUnbalGP( Map *map, MapEl *element );
void mapListAddBefore( Map *map, MapEl *next_el, MapEl *new_el );
void mapListAddAfter( Map *map, MapEl *prev_el, MapEl *new_el );
void mapDeleteChildrenOf( Map *map, MapEl *n );
void mapRecalcHeights( Map *map, MapEl *start );
void mapReplaceEl( Map *map, MapEl *element, MapEl *replacement );
void mapRemoveEl( Map *map, MapEl *element, MapEl *filler );
void mapAttachRebal( Map *map, MapEl *element, MapEl *parentEl, MapEl *lastLess );

struct Accum
{
	/* Must overlay Tree. */
	short id;
	unsigned short flags;
	long refs;
	Kid *child;

	GenericInfo *genericInfo;

	PdaRun *pdaRun;
	FsmRun *fsmRun;
	Stream *stream;
};

struct Stream
{
	/* Must overlay Tree. */
	short id;
	unsigned short flags;
	long refs;
	Kid *child;

	FILE *file;
	InputStream *in;
};

/*
 * Iterators.
 */

struct TreeIter
{
	TreeIter( const Ref &rootRef, int searchId, Tree **stackRoot ) : 
		rootRef(rootRef), searchId(searchId), 
		stackRoot(stackRoot), stackSize(0)
	{
		ref.kid = 0;
		ref.next = 0;
	}
	
	Ref rootRef;
	Ref ref;
	long searchId;
	Tree **stackRoot;
	long stackSize;
};

/* This must overlay tree iter because some of the same bytecodes are used. */
struct RevTreeIter
{
	RevTreeIter( const Ref &rootRef, int searchId, Tree **stackRoot, int children ) : 
		rootRef(rootRef), searchId(searchId), 
		stackRoot(stackRoot), stackSize(children), kidAtYield(0), children(children)
	{
		ref.kid = 0;
		ref.next = 0;
	}
	
	Ref rootRef;
	Ref ref;
	long searchId;
	Tree **stackRoot;
	long stackSize;

	/* For detecting a split at the leaf. */
	Kid *kidAtYield;
	long children;
	Kid **cur;
};

struct FunctionInfo
{
	const char *name;
	long frameId;
	long argSize;
	long ntrees;
	long frameSize;
};

struct UserIter
{
	/* The current item. */
	Ref ref;
	Tree **stackRoot;
	long argSize;
	long stackSize;
	Code *resume;
	Tree **frame;
	long searchId;
};

void initUserIter( UserIter *userIter, Tree **stackRoot, long argSize, long searchId );


/*
 * Program Data.
 */

struct PatReplInfo
{
	long offset;
	long numBindings;
};

struct PatReplNode
{
	long id;
	long next;
	long child;
	long bindId;
	const char *data;
	long length;
	long ignore;

	/* Just match nonterminal, don't go inside. */
	bool stop;
};

/* FIXME: should have a descriptor for object types to give the length. */

struct LangElInfo
{
	const char *name;
	bool repeat;
	bool list;
	bool literal;
	bool ignore;

	long frameId;

	long objectTypeId;
	long ofiOffset;
	long objectLength;

//	long contextTypeId;
//	long contextLength;

	long termDupId;
	long genericId;
	long markId;
	long captureAttr;
	long numCaptureAttr;
};

struct ObjFieldInfo
{
	int typeId;
};

struct ProdInfo
{
	long length;
	unsigned long lhsId;
	const char *name;
	long frameId;
	bool lhsUpref;
};

struct FrameInfo
{
	Code *codeWV;
	long codeLenWV;
	Code *codeWC;
	long codeLenWC;
	char *trees;
	long treesLen;
};

struct RegionInfo
{
	const char *name;
	long defaultToken;
	long eofFrameId;
};

struct CaptureAttr
{
	long mark_enter;
	long mark_leave;
	long offset;
};

struct RuntimeData
{
	LangElInfo *lelInfo;
	long numLangEls;

	ProdInfo *prodInfo;
	long numProds;

	RegionInfo *regionInfo;
	long numRegions;

	Code *rootCode;
	long rootCodeLen;

	FrameInfo *frameInfo;
	long numFrames;

	FunctionInfo *functionInfo;
	long numFunctions;

	PatReplInfo *patReplInfo;
	long numPatterns;

	PatReplNode *patReplNodes;
	long numPatternNodes;

	GenericInfo *genericInfo;
	long numGenerics;

	const char **litdata;
	long *litlen;
	Head **literals;
	long numLiterals;

	CaptureAttr *captureAttr;
	long numCapturedAttr;

	FsmTables *fsmTables;
	PdaTables *pdaTables;
	int *startStates;
	int *eofLelIds;
	int *parserLelIds;
	long numParsers;

	long globalSize;

	long firstNonTermId;

	long integerId;
	long stringId;
	long anyId;
	long eofId;
	long noTokenId;
};

struct PdaTables
{
	/* Parser table data. */
	int *indicies;
	int *keys;
	unsigned int *offsets;
	unsigned int *targs;
	unsigned int *actInds;
	unsigned int *actions;
	int *commitLen;
	int *tokenRegionInds;
	int *tokenRegions;

	int numIndicies;
	int numKeys;
	int numStates;
	int numTargs;
	int numActInds;
	int numActions;
	int numCommitLen;
	int numRegionItems;

	RuntimeData *rtd;
};

bool makeReverseCode( CodeVect *all, CodeVect &reverseCode );

typedef RtVector<Tree*> Bindings;

struct PdaRun
{
	PdaRun( Program *prg, PdaTables *tables, FsmRun *fsmRun, int parserId,
			long stopTarget, bool revertOn )
	:
		prg(prg),
		tables(tables), 
		fsmRun(fsmRun),
		parserId(parserId), 
		stopParsing(false),
		stopTarget(stopTarget),
		queue(0),
		queueLast(0),
		revertOn(revertOn),
		context(0),
		consumed(0),
		targetConsumed(-1)
	{
	}

	int numRetry;
	Kid *stackTop;
	int errCount;
	int cs;
	int nextRegionInd;

	/* Offset can be used to look at the next nextRegionInd. */
	int getNextRegion( int offset = 0 )
		{ return tables->tokenRegions[nextRegionInd+offset]; }

	Program *prg;
	PdaTables *tables;
	FsmRun *fsmRun;
	int parserId;

	long stackTopTarget();
	void commitKid( Tree **root, Kid *lel );
	void commit();
	bool isParserStopFinished();
	void match( Kid *tree, Kid *pattern );

	Kid *extractIgnore();

	/* Report an error encountered by the parser. */
	ostream &parseError( int tokId, Tree *tree );

	/* Reused. */
	CodeVect reverseCode;
	CodeVect *allReverseCode;

	bool stopParsing;
	long stopTarget;

	Kid *accumIgnore;
	Kid *queue, *queueLast;

	Bindings bindings;

	bool revertOn;

	Tree *context;
	void clearContext( Tree **sp );

	//bool fragStop;
	bool stop;

	long consumed;
	long targetConsumed;
};

void cleanParser( Tree **root, PdaRun *pdaRun );
void ignore( PdaRun *pdaRun, Tree *tree );
void parseToken( Tree **sp, PdaRun *pdaRun, FsmRun *fsmRun, InputStream *inputStream, Kid *input );
long undoParse( Tree **sp, PdaRun *pdaRun, FsmRun *fsmRun, InputStream *inputStream, Tree *tree );
void xml_print_list( RuntimeData *runtimeData, Kid *lel, int depth );
ostream &parseError( InputStream *inputStream, FsmRun *fsmRun, PdaRun *pdaRun, int tokId, Tree *tree );
void initPdaRun( PdaRun *pdaRun, Tree *tree );

#endif /* _PDARUN_H */
