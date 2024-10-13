/*****************************************************************************/
/* ファイル操作に関する処理						     */
/*									     */
/*	仕様の詳細は、ヘッダファイル file-op.h 参照			     */
/*									     */
/*****************************************************************************/
#ifdef _CORES3
#include <SD.h>
//#include <SPIFFS.h>

#include <M5Unified.h>
#elif defined _ATOMS3R
#include <M5GFX.h>
#include <SPIFFS.h>
#else
#include <M5Stack.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quasi88.h"
#include "file-op.h"
#include "initval.h"
extern "C"
{
#include "graph.h"
}
/*****************************************************************************/

static char *dir_cwd;	/* デフォルトのディレクトリ (カレント)		*/
static char *dir_rom;	/* ROMイメージファイルの検索ディレクトリ	*/
static char *dir_disk;	/* DISKイメージファイルの検索ディレクトリ	*/
static char *dir_tape;	/* TAPEイメージファイルの基準ディレクトリ	*/
static char *dir_snap;	/* 画面スナップショットファイルの保存先		*/
static char *dir_state;	/* サスペンドファイルの保存先			*/
static char *dir_g_cfg;	/* 共通設定ファイルのディレクトリ		*/
static char *dir_l_cfg;	/* 個別設定ファイルのディレクトリ		*/

/****************************************************************************
 * 各種ディレクトリの取得と設定
 *****************************************************************************/
const char *osd_dir_cwd  (void) { return dir_cwd;   }
const char *osd_dir_rom  (void) { return dir_rom;   }
const char *osd_dir_disk (void) { return dir_disk;  }
const char *osd_dir_tape (void) { return dir_tape;  }
const char *osd_dir_snap (void) { return dir_snap;  }
const char *osd_dir_state(void) { return dir_state; }
const char *osd_dir_gcfg (void) { return dir_g_cfg; }
const char *osd_dir_lcfg (void) { return dir_l_cfg; }

static int set_new_dir(const char *newdir, char **dir)
{
    char *p = (char*)malloc(strlen(newdir) + 1);
    if (p) {
	strcpy(p, newdir);
	free(*dir);
	*dir = p;
	return TRUE;
    }
    return FALSE;
}

int osd_set_dir_cwd  (const char *d) { return set_new_dir(d, &dir_cwd);   }
int osd_set_dir_rom  (const char *d) { return set_new_dir(d, &dir_rom);   }
int osd_set_dir_disk (const char *d) { return set_new_dir(d, &dir_disk);  }
int osd_set_dir_tape (const char *d) { return set_new_dir(d, &dir_tape);  }
int osd_set_dir_snap (const char *d) { return set_new_dir(d, &dir_snap);  }
int osd_set_dir_state(const char *d) { return set_new_dir(d, &dir_state); }
int osd_set_dir_gcfg (const char *d) { return set_new_dir(d, &dir_g_cfg); }
int osd_set_dir_lcfg (const char *d) { return set_new_dir(d, &dir_l_cfg); }







/****************************************************************************
 * ファイル名に使用されている漢字コードを取得
 *		0 … ASCII のみ
 *		1 … 日本語EUC
 *		2 … シフトJIS
 *****************************************************************************/
int	osd_kanji_code(void)
{
    return 0;	/* ASCIIのみ */
}



/****************************************************************************
 * ファイル操作
 *
 * OSD_FILE *osd_fopen(int type, const char *path, const char *mode)
 * int	osd_fclose(OSD_FILE *stream)
 * int	osd_fflush(OSD_FILE *stream)
 * int	osd_fseek(OSD_FILE *stream, long offset, int whence)
 * long	osd_ftell(OSD_FILE *stream)
 * void	osd_rewind(OSD_FILE *stream)
 * size_t osd_fread(void *ptr, size_t size, size_t nobj, OSD_FILE *stream)
 * size_t osd_fwrite(const void *ptr,size_t size,size_t nobj,OSD_FILE *stream)
 * int	osd_fputc(int c, OSD_FILE *stream)
 * int	osd_fgetc(OSD_FILE *stream)
 * char	*osd_fgets(char *str, int size, OSD_FILE *stream)
 * int	osd_fputs(const char *str, OSD_FILE *stream)
 *****************************************************************************/


/*
 * 全てのファイルに対して排他制御したほうがいいと思うけど、面倒なので、
 * ディスク・テープのイメージに関してのみ、多重にオープンしないようにする。
 * 機種非依存の方法はないので、ここではファイル名で区別することにする。(危険?)
 *
 * osd_fopen が呼び出されたときに、ファイル名を保持しておき、
 * すでに開いているファイルのファイル名と一致しないかをチェックする。
 * ここで、ディスクイメージファイルの場合は、すでに開いているファイルの
 * ファイルポインタを返し、他の場合はオープン失敗として NULL を返す。
 */

struct OSD_FILE_STRUCT {
    File	sdFile;			/* !=NULL なら使用中	*/
    bool    useFlag;			/* ==true なら使用中	*/
    int		type;			/* ファイル種別		*/
    char	*path;			/* ファイル名		*/
    char	mode[4];		/* 開いた際の、モード	*/
};

#define	MAX_STREAM	8
static	OSD_FILE	osd_stream[ MAX_STREAM ];

//static File fileIndex[MAX_STREAM];

void initialize(){
    for(int i = 0; i < MAX_STREAM;i++){
        osd_stream[i].useFlag = false;
    }
}


OSD_FILE *osd_fopen(int type, const char *path, const char *mode)
{
  waitDrawing();
    int i;
    OSD_FILE	*st;

    st = NULL;
    for (i=0; i<MAX_STREAM; i++) {	/* 空きバッファを探す */
	    if (osd_stream[i].useFlag == false) {		/* fp が NULL なら空き */
	        st = &osd_stream[i];
	        break;
	    }
    }

    if (st == NULL){
        return NULL;		/* 空きがなければ NG */
    }
    st->path = NULL;

    switch (type) {

        case FTYPE_DISK:		/* "r+b" , "rb"	*/
        case FTYPE_TAPE_LOAD:	/* "rb" 	*/
        case FTYPE_TAPE_SAVE:	/* "ab"		*/
        case FTYPE_PRN:		/* "ab"		*/
        case FTYPE_COM_LOAD:	/* "rb"		*/
        case FTYPE_COM_SAVE:	/* "ab"		*/

    	/* すでに開いているファイルかどうかをチェックする */
    	for (i=0; i<MAX_STREAM; i++) {
	        if (osd_stream[i].useFlag == false) {
		        if (osd_stream[i].path   &&
    		        strcmp(osd_stream[i].path, path) == 0) {

    		        /* DISKの場合かつ同じモードならばそれを返す */
		            if (type == FTYPE_DISK                   &&
		    	        osd_stream[i].type == type           &&
			            strcmp(osd_stream[i].mode, mode) == 0) {
               			return &osd_stream[i];
        		    } else {
                        Serial.println("4");
		            	/* DISK以外、ないしモードが違うならばNG */
                        graph_restartDrawing();
			            return NULL;
		            }
		        }
	        }
	    }
		/* ファイル名保持用のバッファを確保 */
	    st->path = (char*)malloc(strlen(path) + 1);
	    if (st->path == NULL) {
            graph_restartDrawing();
    	    return NULL;
    	}
	    /* FALLTHROUGH */


        default:
            Serial.print("Path:");
            Serial.println(path);
            String stringPath = String(path);
#if defined _ATOMS3R
            if(strchr(mode,'+')!=0){
                st->sdFile = SPIFFS.open(stringPath, "r+");
            }else if(strchr(mode,'a')!=0){
                st->sdFile = SPIFFS.open(stringPath, FILE_APPEND);
            }else if(strchr(mode,'w')!=0){
                st->sdFile = SPIFFS.open(stringPath, FILE_WRITE);
            }else{
                st->sdFile = SPIFFS.open(stringPath, FILE_READ);
            }
#else
            if(strchr(mode,'+')!=0){
                st->sdFile = SD.open(stringPath, "r+");
            }else if(strchr(mode,'a')!=0){
                st->sdFile = SD.open(stringPath, FILE_APPEND);
            }else if(strchr(mode,'w')!=0){
                st->sdFile = SD.open(stringPath, FILE_WRITE);
            }else{
                st->sdFile = SD.open(stringPath, FILE_READ);
            }
#endif
	        if (st->sdFile) {
        	    st->type = type;
        	    if (st->path){
            		strcpy(st->path, path);
                }
	            strncpy(st->mode, mode, sizeof(st->mode));
                st->useFlag = true;
                graph_restartDrawing();
                Serial.println("open Success!");
	            return st;
            } else {
                Serial.println("open Fail!");
        	    if (st->path) {
                    free(st->path);
        		    st->path = NULL;
           	    }
                graph_restartDrawing();
	            return NULL;
            }
    }
}



int	osd_fclose(OSD_FILE *stream)
{
  waitDrawing();
  graph_stopDrawing();
    stream->sdFile.close();
  	stream->useFlag = false;
   	if (stream->path) {
        free(stream->path);
        stream->path = NULL;
    }
    graph_restartDrawing();
    return 0;
}



int	osd_fflush(OSD_FILE *stream)
{
  waitDrawing();
  graph_stopDrawing();
    if (stream == NULL) {
        graph_restartDrawing();
        return fflush(NULL);
    }
    stream->sdFile.flush();
    graph_restartDrawing();
    return EOF;
}



int	osd_fseek(OSD_FILE *stream, long offset, int whence)
{
  waitDrawing();
  graph_stopDrawing();
    SeekMode seekMode;
    if(whence == SEEK_END){
        seekMode = SeekEnd;
    }else if(whence == SEEK_CUR){
        seekMode = SeekCur;
    }else{
        seekMode = SeekSet;
    }

    stream->sdFile.seek(offset, seekMode);
    graph_restartDrawing();
    return 0;
}



long	osd_ftell(OSD_FILE *stream)
{
  waitDrawing();
  graph_stopDrawing();
  long position = stream->sdFile.position();  
  graph_restartDrawing();
  return position;
}


void	osd_rewind(OSD_FILE *stream)
{
  waitDrawing();
  graph_stopDrawing();
    (void)osd_fseek(stream, 0L, SEEK_SET);
    osd_fflush(stream);
  graph_restartDrawing();
}

size_t	osd_fread(void *ptr, size_t size, size_t nobj, OSD_FILE *stream)
{
  waitDrawing();
  graph_stopDrawing();
    size_t readByte = 0;
    if(stream->sdFile.available()){
      readByte = stream->sdFile.read((byte*)ptr, nobj);
    }
    graph_restartDrawing();
    return (readByte / size);
}



size_t	osd_fwrite(const void *ptr, size_t size, size_t nobj, OSD_FILE *stream)
{
  waitDrawing();
  graph_stopDrawing();
    int writeByte = stream->sdFile.write((byte*)ptr, size * nobj);
    //Serial.printf("size:%d  nobj:%d retSize:%d", size, nobj, writeByte);
    graph_restartDrawing();
    return writeByte / size;
}

int	osd_fputc(int c, OSD_FILE *stream)
{
  waitDrawing();
  graph_stopDrawing();
  int result = stream->sdFile.write(c);
  graph_restartDrawing();
  return result;
}


int	osd_fgetc(OSD_FILE *stream)
{
  waitDrawing();
  graph_stopDrawing();
    uint8_t readData;
    if(stream->sdFile.available()){
      stream->sdFile.read(&readData, 1);
      graph_restartDrawing();
      return readData;
    }
    graph_restartDrawing();
    return EOF;
}


char *osd_fgets(char *str, int size, OSD_FILE *stream)
{
    waitDrawing();
    graph_stopDrawing();
    if(stream->sdFile.available()){
	    stream->sdFile.readBytes(str, size);
        graph_restartDrawing();
        return str;
    }
    graph_restartDrawing();
    return NULL;
}


int	osd_fputs(const char *str, OSD_FILE *stream)
{
    waitDrawing();
    graph_stopDrawing();
    int length = strnlen(str, 1024);
    int result = stream->sdFile.write((uint8_t*)str, length); 
    graph_restartDrawing();
    return result;
}



/****************************************************************************
 * ディレクトリ閲覧
 *****************************************************************************/

/*---------------------------------------------------------------------------
 * T_DIR_INFO *osd_opendir(const char *filename)
 *	ディレクトリ処理は機種依存なので、ここでは NULL (エラー) を返す。
 *---------------------------------------------------------------------------*/
T_DIR_INFO	*osd_opendir(const char *filename)
{
    return NULL;
}



/*---------------------------------------------------------------------------
 * T_DIR_ENTRY *osd_readdir(T_DIR_INFO *dirp)
 *	ディレクトリ処理は機種依存なので、ここでは NULL (エラー) を返す。
 *---------------------------------------------------------------------------*/
T_DIR_ENTRY	*osd_readdir(T_DIR_INFO *dirp)
{
    return NULL;
}


/*---------------------------------------------------------------------------
 * void osd_closedir(T_DIR_INFO *dirp)
 *	ディレクトリ処理は機種依存なので、この関数はダミー
 *---------------------------------------------------------------------------*/
void		osd_closedir(T_DIR_INFO *dirp)
{
}



/****************************************************************************
 * パス名の操作
 *****************************************************************************/

/*---------------------------------------------------------------------------
 * int	osd_path_normalize(const char *path, char resolved_path[], int size)
 *	ファイル名やパスについての処理は機種依存なので、
 *	path をそのまま resolved_path にセットして返す
 *---------------------------------------------------------------------------*/
int	osd_path_normalize(const char *path, char resolved_path[], int size)
{
    if (strlen(path) < size) {
	strcpy(resolved_path, path);
	return TRUE;
    }

    return FALSE;
}



/*---------------------------------------------------------------------------
 * int	osd_path_split(const char *path, char dir[], char file[], int size)
 *	ファイル名やパスについての処理は機種依存なので、
 *	path をそのまま file にセットして返す
 *---------------------------------------------------------------------------*/
int	osd_path_split(const char *path, char dir[], char file[], int size)
{
    if (strlen(path) < size) {
	dir[0] = '\0';
	strcpy(file, path);
	return TRUE;
    }

    return FALSE;
}



/*---------------------------------------------------------------------------
 * int	osd_path_join(const char *dir, const char *file, char path[], int size)
 *---------------------------------------------------------------------------*/
int	osd_path_join(const char *dir, const char *file, char path[], int size)
{
    if (strlen(dir) + strlen(file) < size) {
        sprintf(path,"%s%s",dir,file);
	    return TRUE;
    }

    return FALSE;
}



/****************************************************************************
 * ファイル属性の取得
 ****************************************************************************/

int	osd_file_stat(const char *pathname)
{
  waitDrawing();
  graph_stopDrawing();
    FILE *fp;

    if ((fp = fopen(pathname, "r"))) {	/* ファイルとして開く	*/

	fclose(fp);				/* 成功したらファイル	*/
    graph_restartDrawing();
	return FILE_STAT_FILE;

    } else {				/* 失敗したら存在しない	*/

	return FILE_STAT_NOEXIST;

	/* ディレクトリかもしれないし、アクセス許可が無いのかも知れない */
	/* でも、これらは機種依存なのでとりあえず '存在しない' を返す。	*/

	/* なお、'存在しない' を返すと、この後にファイルを新規に作成	*/
	/* しようとするかもしれない。(fopen(pathname, "w") などで)	*/
	/* でも、ディレクトリが存在したり、アクセス許可の無いファイルが	*/
	/* 存在する場合は、新規作成に失敗するので大丈夫、だよね？	*/
    }
}






/****************************************************************************
 * int	osd_environment(void)
 *
 *	この関数は、起動後に1度だけ呼び出される。
 *	正常終了時は真を、 malloc に失敗したなど異常終了時は偽を返す。
 *
 ****************************************************************************/

int	osd_file_config_init(void)
{
	/* ワークを確保*/

    dir_cwd   = (char *)malloc(1);
    dir_rom   = (char *)malloc(1);
    dir_disk  = (char *)malloc(1);
    dir_tape  = (char *)malloc(1);
    dir_snap  = (char *)malloc(1);
    dir_state = (char *)malloc(1);
    dir_g_cfg = (char *)malloc(1);
    dir_l_cfg = (char *)malloc(1);


    if (! dir_cwd  || ! dir_rom   || ! dir_disk  || ! dir_tape ||
	! dir_snap || ! dir_state || ! dir_g_cfg || ! dir_l_cfg)  return FALSE;


	/* カレントワーキングディレクトリ名 (CWD) を設定する */
    dir_cwd[0] = '\0';
#ifdef _ATOMS3R
	/* ROMディレクトリを設定する */
    osd_set_dir_rom("/PC88ROM/");

	/* DISKディレクトリを設定する */
    osd_set_dir_disk("/PC88ROM/DISK/");

	/* TAPEディレクトリを設定する */
    osd_set_dir_tape("/PC88ROM/TAPE/");

	/* SNAPディレクトリを設定する */
    osd_set_dir_snap("/PC88ROM/SNAP/");

	/* STATEディレクトリを設定する */
    osd_set_dir_state("/PC88ROM/");

#endif
	/* ROMディレクトリを設定する */
    osd_set_dir_rom("/PC88ROM/");

	/* DISKディレクトリを設定する */
    osd_set_dir_disk("/PC88ROM/DISK/");

	/* TAPEディレクトリを設定する */
    osd_set_dir_tape("/PC88ROM/TAPE/");

	/* SNAPディレクトリを設定する */
    osd_set_dir_snap("/PC88ROM/SNAP/");

	/* STATEディレクトリを設定する */
    osd_set_dir_state("/PC88ROM/");

	/* 全体設定ディレクトリを設定する */
    dir_g_cfg[0] = '\0';

	/* 個別設定ディレクトリを設定する */

    dir_l_cfg[0] = '\0';



    return TRUE;
}


/****************************************************************************
 * int	osd_file_config_exit(void)
 *
 *	この関数は、終了後に1度だけ呼び出される。
 *
 ****************************************************************************/
void	osd_file_config_exit(void)
{
    if (dir_cwd)   free(dir_cwd);
    if (dir_rom)   free(dir_rom);
    if (dir_disk)  free(dir_disk);
    if (dir_tape)  free(dir_tape);
    if (dir_snap)  free(dir_snap);
    if (dir_state) free(dir_state);
    if (dir_g_cfg) free(dir_g_cfg);
    if (dir_l_cfg) free(dir_l_cfg);
}
