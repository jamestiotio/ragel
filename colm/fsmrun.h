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

#ifndef _FSMRUN_H
#define _FSMRUN_H

#include "pdarun.h"
#include "input.h"

struct GenAction;
struct KlangEl;
struct PdaRun;
struct ParseData;
typedef struct _Kid Kid;
struct Pattern;
struct PatternItem;
struct Replacement;
struct ReplItem;

#include "fsmrun2.h"

struct FsmRun
{
	Program *prg;
	FsmTables *tables;

	RunBuf *runBuf;

	/* FsmRun State. */
	long region, cs, act;
	char *tokstart, *tokend;
	char *p, *pe, *peof;
	bool returnResult;
	char *mark[MARK_SLOTS];
	long matchedToken;

	InputStream *haveDataOf;
	Tree *curStream;
};

void execAction( FsmRun *fsmRun, GenAction *genAction );
void fsmExecute( FsmRun *fsmRun, InputStream *inputStream );
void initFsmRun( FsmRun *fsmRun, Program *prg );
void initInputStream( InputStream *in );
void sendQueuedTokens( Tree **sp, PdaRun *pdaRun, FsmRun *fsmRun, InputStream *inputStream );
void sendHandleError( Tree **sp, PdaRun *pdaRun, FsmRun *fsmRun, InputStream *inputStream, Kid *input );
Head *extractMatch( Program *prg, FsmRun *fsmRun, InputStream *inputStream );
void queueBackTree( Tree **sp, PdaRun *pdaRun, FsmRun *fsmRun, InputStream *inputStream, Kid *input );
void parseLoop( Tree **sp, PdaRun *pdaRun, FsmRun *fsmRun, InputStream *inputStream );
long scan_token( PdaRun *pdaRun, FsmRun *fsmRun, InputStream *inputStream );

Head *streamPull( Program *prg, FsmRun *fsmRun, InputStream *inputStream, long length );
void stream_push_text( InputStream *inputStream, const char *data, long length );
void streamPushTree( InputStream *inputStream, Tree *tree, bool ignore );
void undoStreamPull( FsmRun *fsmRun, InputStream *inputStream, const char *data, long length );

void undoStreamPush( Program *prg, Tree **sp, InputStream *inputStream, long length );
void undoStreamAppend( Program *prg, Tree **sp, InputStream *inputStream, long length );

void send_named_lang_el( Tree **sp, PdaRun *pdaRun, FsmRun *fsmRun, InputStream *inputStream );
Kid *makeToken( PdaRun *pdaRun, FsmRun *fsmRun, InputStream *inputStream, int id,
		Head *tokdata, bool namedLangEl, int bindId );

void newToken( PdaRun *pdaRun, FsmRun *fsmRun );
long scan_token( PdaRun *pdaRun, FsmRun *fsmRun, InputStream *inputStream );
void incrementConsumed( PdaRun *pdaRun );
void ignore( PdaRun *pdaRun, Tree *tree );

#endif
