/************************************************************************
*  MyBB(r) - My Business Basic Interpreter                              *
*  -------------------------------------------------------------------  *
*  Program:    functions.c                                              *
************************************************************************/

#include <math.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/utsname.h>
#include <pwd.h>
#include "mybb.h"

extern mybbfloat divide(mybbfloat flt1, mybbfloat flt2);
extern mybbfloat add(mybbfloat flt1, mybbfloat flt2);
extern int itoh(byte num, char *pt);

mybbstring fnc_pad(char *padchar, char *type, int newlength, mybbstring original);
mybbstring fnc_not(mybbstring in);
mybbstring fnc_and(mybbstring in1, mybbstring in2);
mybbstring fnc_xor(mybbstring in1, mybbstring in2);
mybbstring fnc_ior(mybbstring in1, mybbstring in2);
mybbstring fnc_chr(int num);
mybbstring fnc_day(void);
mybbstring fnc_cds(void);
mybbstring fnc_inf(int num1, int num2);
mybbstring fnc_swp(mybbstring str1, mybbstring str2);
mybbstring fnc_lrc(mybbstring str);
mybbstring fnc_gap(mybbstring str);
mybbstring fnc_crc(mybbstring str, mybbstring seed);
mybbstring fnc_hsh(mybbstring str, mybbstring seed);
mybbstring fnc_cvt(mybbstring str, unsigned int opt);
mybbstring fnc_stp(mybbstring str, unsigned int opt);
mybbstring fnc_rev(mybbstring s);
mybbstring fnc_sdx(mybbstring str);
mybbstring fnc_ntd(mybbfloat flt);
mybbstring fnc_pgm(program *pgm, byte line);
mybbstring fnc_hta(mybbstring in);
mybbstring fnc_ucs(mybbstring s);
mybbstring fnc_lcs(mybbstring s);
mybbstring fnc_ath(mybbstring c);
mybbfloat fnc_tim(void);
mybbfloat fnc_tms(void);
int fnc_err(statement *stmt, int start);
int fnc_tcb(int num);
long fnc_rti(const char *str);

struct utsname Uname;

struct numeral {
      long val;
      int  ch;
};

static struct numeral numerals[] = {
      {    1L, 'I' },
      {    5L, 'V' },
      {   10L, 'X' },
      {   50L, 'L' },
      {  100L, 'C' },
      {  500L, 'D' },
      { 1000L, 'M' }
};

void exec_function(program *pgm, statement *stmt, int pos)
{
	int x = 0, y = 0; 
	double z = 0;
	mybbstring c, d, e;
	filetbl *tmpfile;
	mybbfloat flt, flt2;  

	switch (stmt->metalist[pos].operation) 
	{
		case FNC_FPT:
			flt = fltexecpop();
			fltexecpush(dbl2flt(flt2dbl(flt)-flt2int(flt)));
			break;
	
		case VAR_ARGC:
			fltexecpush(int2flt(envinfo.argc));
			break;

		case VAR_SSN:
			strexecpush(SSNID);
			break;

		case VAR_UNT:
			tmpfile = envinfo.firstfile;
			if (tmpfile == NULL) 
			{ 
				fltexecpush(int2flt(1)); break; 
			}
			
			x = 1;
			do 
			{
				if (tmpfile->channel != x)
				{ 
					fltexecpush(int2flt(x)); 
					break; 
				} else 
					x = tmpfile->channel+1;

				tmpfile = tmpfile->nextfile;
			} while (tmpfile != NULL);
			fltexecpush(int2flt(x));
			break;
		
		case VAR_DATESTRINGS:
			break;
		
		case VAR_CDS:
			strexecpush3(fnc_cds());
			break;
		
		case VAR_DAY:
			strexecpush3(fnc_day());
			break;
		
		case VAR_SYS:
			strexecpush(SYSNAME);
			//strexecpush(SYSNAME " " VERSION ";" SYSID);
			break;
		
		case VAR_SEP:
			strexecpush("\x8A");
			break;
			
		case VAR_DLM:
			strexecpush(SYSDELIM);
		
		case VAR_EOM:
			strexecpush("\x0D");
			break;

		case VAR_ESC:
			strexecpush("\x1B");
			break;

		case VAR_QUO:
			strexecpush("\x22");
			break;
		
		case VAR_PGN: 
			if (pgm->filename)
				strexecpush(pgm->filename);
			else 
				strexecpush("No program currently loaded");
			break;

		case VAR_DIR:
			#ifdef WIN32
				strexecpush((char *)strReplace(strcat(getcwd(0,0),"/"),'/',SYSDELIM));
			#endif

			#ifdef LINUX
				strexecpush((char *)strcat(getcwd(0,0),"/"));
			#endif
			break;

		case VAR_ERR:
			fltexecpush(int2flt(envinfo.lasterror));
			break;

		case VAR_ERS:
			fltexecpush(int2flt(fnc_tcb(5)));
			break;

		case VAR_OCH:
			if (!envinfo.firstfile) 
			{ 
				strexecpush("\0"); 
				break;
			}

			tmpfile = envinfo.firstfile;
			c.str = SafeMalloc(1024*64);
			y = 0;
			do 
			{
				y += sprintf(c.str+y, "%.4X", tmpfile->channel);
				tmpfile = tmpfile->nextfile;
			} while (tmpfile != NULL);
			
			c.len = y;
			realloc(c.str, c.len);
			strexecpush3(fnc_ath(c));
			break;
		
		case FNC_ARG: 
			x = flt2int(fltexecpop());
			if (x >= envinfo.argc) 
				rterror(ERR_INVALID_PARAMETER);
			else 
				strexecpush(envinfo.argv[x]);
			break;
		
		case FNC_EXP: 
			z = exp(flt2dbl(fltexecpop()));      
			fltexecpush(dbl2flt(z));
			break;
		
		case FNC_ABS:
			flt = fltexecpop();
			if (flt.mantisa.i < 0) 
				flt.mantisa.i = -flt.mantisa.i;

			fltexecpush(flt);
			break;
		
		case FNC_SGN:
			x = flt2int(fltexecpop());
			if (x == 0) 
				flt.mantisa.i = 0; 
			else if (x > 0) flt.mantisa.i = 1;
			else 
				flt.mantisa.i = -1;
			
			flt.exp = 0;
			fltexecpush(flt);
			break;
		
		case FNC_MOD:
			z = flt2dbl(fltexecpop());
			fltexecpush(dbl2flt(fmod(flt2dbl(fltexecpop()), z))); //y % x);
			break;
		
		case FNC_INT:
			x = flt2int(fltexecpop());
			fltexecpush(int2flt(x));
			break;
		
		case FNC_SQR:
			z = sqrt(flt2dbl(fltexecpop()));      
			fltexecpush(dbl2flt(z));
			break;
		
		case FNC_NLG:
			z = log(flt2dbl(fltexecpop()));
			fltexecpush(dbl2flt(z));
			break;
		
		case FNC_LOG:
			z = log10(flt2dbl(fltexecpop()));
			fltexecpush(dbl2flt(z));
			break;
		
		case FNC_PGM:
			strexecpush3(fnc_pgm(pgm, flt2int(fltexecpop())));
			break;
		
		case FNC_ERM:
			x = (int)flt2dbl(fltexecpop());
			for (y=0; errortable[y].errortext; y++) 
			{
				if (x == errortable[y].errorcode) 
				{ 
					if (strlen(errortable[y].errortext) == 0) 
						rterror(ERR_INTEGER_RANGE);

					strexecpush(errortable[y].errortext); 
					break; 
				}
			}
			break;
		
		case FNC_STL:
		
		case FNC_LEN:
			c = strexecpop();
			fltexecpush(int2flt(c.len));
			break;
		
		case FNC_SIN:
			z = sin(flt2dbl(fltexecpop()));
			fltexecpush(dbl2flt(z));
			break;
		
		case FNC_COS:
			z = cos(flt2dbl(fltexecpop()));
			fltexecpush(dbl2flt(z));
			break;
		
		case FNC_TAN:
			z = tan(flt2dbl(fltexecpop()));
			fltexecpush(dbl2flt(z));
			break;
		
		case FNC_ASN:
			z = asin(flt2dbl(fltexecpop()));
			fltexecpush(dbl2flt(z));
			break;
		
		case FNC_ATN:
			z = atan(flt2dbl(fltexecpop()));
			fltexecpush(dbl2flt(z));
			break;
		
		case FNC_ACS:
			z = acos(flt2dbl(fltexecpop()));
			fltexecpush(dbl2flt(z));
			break;
		
		case FNC_NOT:
			strexecpush3(fnc_not(strexecpop()));
			break;
		
		case FNC_IOR:
			strexecpush3(fnc_ior(strexecpop(), strexecpop()));
			break;
		
		case FNC_XOR:
			strexecpush3(fnc_xor(strexecpop(), strexecpop()));
			break;
		
		case FNC_AND:
			strexecpush3(fnc_and(strexecpop(), strexecpop()));
			break;
		
		case FNC_ATH:
			c = fnc_ath(strexecpop());
			strexecpush3(c);
			break;
		
		case VAR_TIM:
			fltexecpush(fnc_tim());
			break;

		case VAR_TMS:
			fltexecpush(fnc_tms());
			break;

		case FNC_ASC:
			c = strexecpop();
			if (c.len == 0) 
				rterror(ERR_STRING_SIZE);

			fltexecpush(int2flt((unsigned int)c.str[0]));
			break;
		
		case FNC_CHR:
			strexecpush3(fnc_chr(flt2int(fltexecpop())));
			break;
			
		case FNC_INF:
			strexecpush3(fnc_inf(flt2int(fltexecpop()), flt2int(fltexecpop())));
			break;
			
		case FNC_NMV:
			c = strexecpop();
			for (x=0; x<c.len; x++) 
			{
				if (!isdigit(c.str[x])&&!isspace(c.str[x])&& c.str[x] != '.') 
				{
					fltexecpush(int2flt(0));
					return;
				}
			}
			fltexecpush(int2flt(1));
			break;
		
		case FNC_RTI:
			c = strexecpop();
			fltexecpush(int2flt(fnc_rti(c.str)));
			break;

		case FNC_NUM:
			c = strexecpop();
			fltexecpush(asc2flt(c.str));
			break;

		case FNC_STR:
			strexecpush(flt2asc(fltexecpop()));
			break;
		
		case FNC_SWP:
			if (stmt->metalist[pos].intarg > 1) 
			{ 
				c = strexecpop();
				strexecpush3(fnc_swp(strexecpop(), c));
			} else {
				d.len = 0; 
				strexecpush3(fnc_swp(strexecpop(), d));
			}

			break;
		
		case FNC_LRC:
			strexecpush3(fnc_lrc(strexecpop()));
			break;

		case FNC_GAP:
			strexecpush3(fnc_gap(strexecpop()));
			break;

		case FNC_ATQ: 
			flt = fltexecpop();
			fltexecpush(dbl2flt(atan(flt2dbl(divide(fltexecpop(), flt)))));
			break;

		case FNC_RND:
		srand(flt2int(fltexecpop()));
		flt = int2flt(rand());
		flt.exp = -(strlen(flt2asc(flt)));
		fltexecpush(flt);
		break;

		case FNC_IND: 
			tmpfile = get_chaninfo(flt2int(fltexecpop()));
			if (tmpfile == NULL) 
				rterror(ERR_FILE_DEVICE_USAGE);
			
			fltexecpush(int2flt(tmpfile->nextrecord));
			break;

		case FNC_CRC:
			strexecpush3(fnc_crc(strexecpop(), strexecpop()));
			break;

		case FNC_HSH:
			strexecpush3(fnc_hsh(strexecpop(), strexecpop()));
			break;

		case FNC_CVT:
			strexecpush3(fnc_cvt(strexecpop(), flt2int(fltexecpop())));
			break;

		case FNC_STP:
			strexecpush3(fnc_stp(strexecpop(), flt2int(fltexecpop())));
			break;

		case FNC_REV:
			strexecpush3(fnc_rev(strexecpop()));
			break;

		case FNC_SDX: 
			strexecpush3(fnc_sdx(strexecpop()));
			break;

		case FNC_NTD:
			fnc_ntd(fltexecpop());
			break;

		case FNC_MAX:
			flt = fltexecpop();
			for (x=0; x<stmt->metalist[pos].intarg-1; x++) 
			{
				flt2 = fltexecpop();
				if (flt2dbl(flt2) > flt2dbl(flt)) 
					flt = flt2;
			}
			fltexecpush(flt);
			break;

		case FNC_MIN:
			flt = fltexecpop();
			for (x=0; x<stmt->metalist[pos].intarg-1; x++)
			{
				flt2 = fltexecpop();
				if (flt2dbl(flt2) < flt2dbl(flt)) 
					flt = flt2;
			}
			fltexecpush(flt);
			break;

		case FNC_HTA:
			c = fnc_hta(strexecpop());
			strexecpush3(c);
			break;

		case FNC_UCS:
			c = fnc_ucs(strexecpop());
			strexecpush3(c);
			break;

		case FNC_LCS:
			c = fnc_lcs(strexecpop());
			strexecpush3(c);
			break;

		case FNC_PAD:
			switch (stmt->metalist[pos].intarg) 
			{
				case 2:
					flt = fltexecpop();
					c = fnc_pad(" ", "L", flt2int(flt), strexecpop());
					break;

				case 3:
					d = strexecpop();
					flt = fltexecpop();
					if (!strcmp(d.str, "R"))
						c = fnc_pad(" ", "R", flt2int(flt), strexecpop());
					else if (!strcmp(d.str, "C"))
						c = fnc_pad(" ", "C", flt2int(flt), strexecpop());
					else if (!strcmp(d.str, "L"))
						c = fnc_pad(" ", "L", flt2int(flt), strexecpop());
					else 
						c = fnc_pad(d.str, "L", flt2int(flt), strexecpop());
					
					break;

				case 4:
					d = strexecpop();
					e = strexecpop();
					flt = fltexecpop();
					c = fnc_pad(d.str, e.str, flt2int(flt), strexecpop());
					break;

				default:
					break;
			}
			strexecpush3(c);
			break;

		case FNC_TCB:
			fltexecpush(int2flt(fnc_tcb(flt2int(fltexecpop()))));
			break;

		case FNC_ERR:
			fltexecpush(int2flt(fnc_err(stmt, pos)));
			break;

		default:
		break;
	}
}

mybbstring fnc_not(mybbstring in)
{
	char *s;
	unsigned int i;
	mybbstring buffer;

	buffer.str = SafeMalloc(in.len);
	buffer.len = in.len;

	s = in.str;
	for (i=0; i<in.len; i++) 
	{
		*buffer.str++ = (char)(~*s++);
	}
	*buffer.str = 0;
	buffer.str-=in.len;

	return buffer;
}

mybbstring fnc_and(mybbstring in1, mybbstring in2)
{
	char *s1, *s2;
	int i;
	mybbstring buffer;

	if (in1.len != in2.len) 
		rterror(ERR_INVALID_PARAMETER);

	buffer.str = SafeMalloc(in1.len);
	buffer.len = in1.len;

	s1 = in1.str;
	s2 = in2.str;
	for (i=0; i<in1.len; i++) 
	{
		*buffer.str++ = *s1++ & *s2++;  
	}
	*buffer.str = 0;
	buffer.str -= in1.len;

	return buffer;
}

mybbstring fnc_xor(mybbstring in1, mybbstring in2)
{
	char *s1, *s2;
	int i;
	mybbstring buffer;

	if (in1.len != in2.len)
		rterror(ERR_INVALID_PARAMETER);

	buffer.str = SafeMalloc(in1.len);
	buffer.len = in1.len;

	s1 = in1.str;
	s2 = in2.str;
	
	for (i=0; i<in1.len; i++) 
	{
		*buffer.str++ = *s1++ ^ *s2++;
	}

	*buffer.str = 0;
	buffer.str -= in1.len;

	return buffer;
}

mybbstring fnc_ior(mybbstring in1, mybbstring in2)
{
	char *s1, *s2;
	int i;
	mybbstring buffer;

	if (in1.len != in2.len) 
		rterror(ERR_INVALID_PARAMETER);

	buffer.str = SafeMalloc(in1.len);
	buffer.len  = in1.len;

	s1 = in1.str;
	s2 = in2.str;
	for (i=0; i<in1.len; i++)
		*buffer.str++ = *s1++ | *s2++;

	*buffer.str = 0;
	buffer.str -= in1.len;

	return buffer;
}

mybbfloat fnc_tim()
	{
	struct tm *tim;
	time_t lt;
	double tmp = 0;

	lt =   time(NULL);
	tim =  localtime(&lt);
	tmp =  (double)tim->tm_hour;
	tmp += (double)tim->tm_min/60.0;
	tmp += (double)tim->tm_sec/3600.0;

	return dbl2flt(tmp);
}

mybbfloat fnc_tms()
{
	struct tm *tim;
	time_t lt;
	double tmp = 0;

	lt =   time(NULL);
	tim =  localtime(&lt);
	tmp =  (double)tim->tm_sec;

	return dbl2flt(tmp);
}

mybbstring fnc_chr(int num)
{
	mybbstring buffer;

	buffer.str = SafeMalloc(1);
	buffer.len = 1;

	if (num > 255) 
		buffer.str[0] = '\0';
	else 
		sprintf(buffer.str, "%c", (char)num);

	return buffer;
}

mybbstring fnc_day(void)
{
	struct tm *tim;
	time_t lt;
	mybbstring buffer;

	lt = time(NULL);
	tim = localtime(&lt);

	buffer.str = SafeMalloc(10);
	buffer.len = 10;

	sprintf(buffer.str, "%.2d/%.2d/%.2d", tim->tm_mon+1, tim->tm_mday, abs(100-tim->tm_year));

	return buffer;
}

mybbstring fnc_cds(void)
{
	struct tm *tim;
	time_t lt;
	mybbstring buffer;
	int x = 0;

	lt = time(NULL);
	tim = localtime(&lt);

	buffer.str = SafeMalloc(100);

	x = sprintf(buffer.str, "%.2d-%.3s-%.4d %.2d:%.2d:%.2d", tim->tm_mday, 
		envinfo.datestrings[tim->tm_mon].name, tim->tm_year+1900, tim->tm_hour, tim->tm_min, tim->tm_sec);

	buffer.len = x-1;

	return buffer;
}

mybbstring fnc_inf(int num1, int num2)
{
	struct passwd *pwd;
	int len, x = 0;
	mybbstring buffer;

	uname (&Uname);
	buffer.str = SafeMalloc(1024);

	switch (num1) {
		case 0:
			if (num2 == 0) 
			{
				x = sprintf(buffer.str, "%s", Uname.sysname);
			} else if (num2 == 1) {
				x = sprintf(buffer.str, "%s %s", Uname.release, Uname.version);
			} else 
				rterror(ERR_INTEGER_RANGE);  

			break;

		case 1:
			if (num2 == 0) 
			{
				x = sprintf(buffer.str, "%s", Uname.machine);
			} else 
				rterror(ERR_INTEGER_RANGE);

			break;

		case 3:
			if (num2 == 0 || num2 == 1) 
			{
				len = num2 ? 8 : 2;
				memset(buffer.str, ' ', len);
				x = sprintf(buffer.str, "%ld", (long)getpid());
			} else if (num2 == 2) 
			{
				x = sprintf(buffer.str, "%s", getlogin());
			} else if (num2 == 3) 
			{
				pwd = getpwnam(getlogin());
				x = sprintf(buffer.str, "%s", pwd->pw_gecos);
			} else 
				rterror(ERR_INTEGER_RANGE);

			break;

		case 4:
			break;

		case 7:
			if (num2 == 0) 
			{
				x = sprintf(buffer.str, VERSION);
			} else if (num2 == 1) 
			{
				x = sprintf(buffer.str, __DATE__);
			} else if (num2 == 2) 
			{
				x = sprintf(buffer.str, __TIME__);
			} else 
				rterror(ERR_INTEGER_RANGE);
			
			break;

		default:
			rterror(ERR_INTEGER_RANGE);
			break;
	}

	buffer.len = x;

	return buffer;
}

mybbstring fnc_swp(mybbstring str1, mybbstring str2)
{
	char *mem;
	unsigned int what, z;
	mybbstring buffer;

	mem = SafeMalloc(str1.len);
	buffer.str = SafeMalloc(str1.len);
	buffer.len = str1.len;

	memcpy(mem, str1.str, str1.len);
	memcpy(buffer.str, str1.str, str1.len);
	 
	if (str2.len == 0) 
		what = (1 | 2 | 4); 
	else
		what = (unsigned int)(*str2.str);

	if (what & 1) 
	{
		for (z = 0; z < (str1.len & ~1); z += 2) 
		{
			buffer.str[z + 1] = mem[z];
			buffer.str[z]     = mem[z + 1];
		}
		
		memcpy(mem, buffer.str, str1.len);
	}

	if (what & 2) 
	{
		for (z = 0; z < (str1.len & ~3); z += 4) 
		{
			buffer.str[z]     = mem[z + 2];
			buffer.str[z + 1] = mem[z + 3];
			buffer.str[z + 2] = mem[z];
			buffer.str[z + 3] = mem[z + 1];
		}

		memcpy(mem, buffer.str, str1.len);
	}

	if (what & 4) 
	{
		for (z = 0; z < (str1.len & ~7); z += 8) 
		{
			buffer.str[z]     = mem[z + 4];
			buffer.str[z + 1] = mem[z + 5];
			buffer.str[z + 2] = mem[z + 6];
			buffer.str[z + 3] = mem[z + 7];
			buffer.str[z + 4] = mem[z];
			buffer.str[z + 5] = mem[z + 1];
			buffer.str[z + 6] = mem[z + 2];
			buffer.str[z + 7] = mem[z + 3];
		}    
	}

	free(mem);

	return buffer;
}

mybbstring fnc_lrc(mybbstring s)
{
	int l;
	char *tmp;
	mybbstring buffer;
	  
	buffer.str = SafeMalloc(s.len); 
	buffer.len = s.len;

	l = s.len;
	*buffer.str = 0;
	tmp = s.str;
	while (l--) *buffer.str ^= *tmp++;

	return buffer;
}

mybbstring fnc_gap(mybbstring s)
	{
	mybbstring tmp;
	int l, ans, i;
	register int bit;
	mybbstring buffer;

	tmp = s;
	l = s.len;
	buffer.str = SafeMalloc(s.len);
	buffer.len = s.len;
	 
	while (l--) 
	{
		bit = 1;
		ans = 0;
		for (i = 0; i < 8; i++) 
		{
			ans += (*tmp.str & bit) ? 1 : 0;
			bit = bit << 1;
		}

		if ((ans & 1) == 0) 
			*tmp.str ^= 0x80;

		tmp.str++;
	}

	tmp.str -= s.len;

	return tmp;
}

mybbstring fnc_crc(mybbstring s, mybbstring seed)
{
	unsigned int rs,i;
	int j,shft;
	char *tmp;
	mybbstring buffer;

	if (seed.len != 2) 
		rterror(ERR_SYNTAX);

	if (s.len < 1) 
		rterror(ERR_SYNTAX);

	buffer.str = SafeMalloc(2);
	buffer.len = 2;
	tmp=str;
	rs=(*seed.str<<8)+*(seed.str+1);
	
	for (i=0; i < s.len; i++) 
	{
			rs^=*tmp++ & 0xff;
			for (j=0;j<8;j++) 
			{
			shft=rs&1;
			rs=rs>>1;
			if (shft) 
				rs^=0xa001;
			}
	}

	sprintf(buffer.str,"%c%c",(rs&0xff00)>>8,rs&0x00ff);

	return buffer;
}

mybbstring fnc_hsh (mybbstring s, mybbstring seed)
{
    unsigned int rs;
    unsigned int i;
    int over;
    char *tmp;
    mybbstring buffer;

    if (seed.len != 2) 
		rterror(ERR_SYNTAX);

    if (s.len < 1) 
		rterror(ERR_SYNTAX);

    buffer.str = SafeMalloc(2);
    buffer.len = 2;
    tmp=s.str;
    rs=(*seed.str<<8)+*(seed.str+1);

    for (i=0; i < s.len; i++) 
	{
      over=(rs&0x8000)?1:0;
      rs=(rs<<1)+over;
      if (!over) 
	  {
		over=(rs&0x8000)?1:0;
 		rs=(rs<<1)+over;
      }

      rs^=*tmp++ & 0xff;
    }
    sprintf(buffer.str,"%c%c",(rs&0xff00)>>8,rs&0x00ff);	

    return buffer;
}

mybbstring fnc_cvt(mybbstring s, unsigned int opt)
{
	char *tmp;
	int length, x = 0;
	char *buffer;

	tmp = s.str;
	length = s.len;
	buffer = SafeMalloc(s.len); 

	if (opt & 1) 
	{
	}

	if (opt & 2) 
	{
		int tlen=0;

		for (x=0; x<strlen(tmp); x++)
		{
			if (s.str[x] == ' ' || s.str[x] == '\t' || s.str[x] == '\0')
			{} else
			{
				tlen++;
			}
		}


		tmp=rmwhite(tmp);
		length=0;

		for (x=0; x<tlen; x++)
		{
			if (s.str[x] == ' ' || s.str[x] == '\t' || s.str[x] == '\0')
			{
				s.str[x]='\0';
				break;
			} else
			{
				length++;
			}
		}
		tmp[length]='\0';
	}

	if (opt & 4) 
	{
	}

	if (opt & 8) 
	{
		while (length>0 && *tmp == ' ')\
			tmp++, length--;

		s.str=tmp;
	}

	if (opt & 16) 
	{
		register int i = 0;
		char *p = tmp;
		int tlen = length;
		while (i < tlen) 
		{
			if (tmp[i] == ' ') 
			{
				*p++ = tmp[i++];
				while (i < tlen && tmp[i] == ' ')
				i++, length--;
				} else 
					*p++ = tmp[i++];
		}
	}

	if (opt & 32) 
	{
		while (*tmp) 
		{
			*tmp = toupper(*tmp);
			tmp++;
		}
	}

	if (opt & 64) 
	{
	}

	if (opt & 128) 
	{
		for (x=length-1; x>0; x--) 
		{
			if (tmp[x]==' ') 
			{
				tmp[x] = '\0';
				length--;
			}
		else 
			break;
		}

		s.len=length;
	}

	if (opt & 256) 
	{
	}

	if (opt & 512) 
	{
	}

	if (opt & 1024)
	{
	}

	if (opt & 2048) 
	{
	}

	if (opt & 4096) 
	{
		while (*tmp) 
		{
			*tmp = tolower(*tmp);
			tmp++;
		}
	}

	if (opt & 8192) 
	{ 
		char *p1, *p2;
		for (p1 = tmp, p2 = tmp + length-1; p2 > p1; ++p1, --p2) 
		{
			*p1 ^= *p2;
			*p2 ^= *p1;
			*p1 ^= *p2;
		}
	}

	for (x=length; x<s.len; x++)
		s.str[x] = '\0';

	return s;
}

mybbstring fnc_stp(mybbstring s, unsigned int opt)
{
	char *tmp;
	int length, x = 0;
	char *buffer;

	tmp = s.str;
	length = s.len;
	buffer = SafeMalloc(s.len); 

	// Strip leading Characters
	if (opt == 0) 
	{
		while (length>0 && *tmp == ' ')
			tmp++, length--;
	}
	  
	// Strip Trailing Characters
	if (opt == 1) 
	{
		for (x=length-1; x>0; x--) 
		{
			if (tmp[x]==' ') 
			{
				tmp[x] = '\0';
				length--;
			}
		else 
			break;
		}
	}

	// Strip Leading & Trailing Characters
	if (opt == 2) 
	{
		while (length>0 && *tmp == ' ')
			tmp++, length--;
		  
		for (x=length-1; x>0; x--) 
		{
			if (tmp[x]==' ') 
			{
				tmp[x] = '\0';
				length--;
			}
		else 
			break;
		}
	}

	// Strip All 
	if (opt == 3)
	{
		int tlen=0;

		for (x=0; x<strlen(tmp); x++)
		{
			if (s.str[x] == ' ' || s.str[x] == '\t' || s.str[x] == '\0')
			{} else
			{
				tlen++;
			}
		}


		tmp=rmwhite(tmp);
		length=0;

		for (x=0; x<tlen; x++)
		{
			if (s.str[x] == ' ' || s.str[x] == '\t' || s.str[x] == '\0')
			{
				s.str[x]='\0';
				break;
			} else
			{
				length++;
			}
		}
		tmp[length]='\0';
	}
	 
	s.str=tmp;
	s.len=length;
	  
	for (x=length; x<s.len; x++)
		s.str[x] = '\0';
		
	return s;
}

mybbstring fnc_rev(mybbstring s)
{
	char *str;
	str = s.str;
	
	char *p1, *p2;

    if (str || *str)
	{
		for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
		{
			*p1 ^= *p2;
			*p2 ^= *p1;
			*p1 ^= *p2;
		}
	}

	s.str=str;
	
	return s;
}

mybbstring fnc_sdx(mybbstring str)
{
	char *sptr = str.str;
	char *rptr;
	mybbstring buffer;

	buffer.str = SafeMalloc(4);
	buffer.len = 4;

	rptr = buffer.str;
	    
	if(*str.str == '\0')
	{
		strcpy(buffer.str,"0000");
		return buffer;
	}
	            
	*(rptr++) = toupper(*(sptr++));
	    
	for(;(rptr - buffer.str) < 4 &&  *sptr != '\0';sptr++)
	{
		switch (toupper(*sptr))
		{
			case 'W':

			case 'H':

			case 'A':

			case 'I':

			case 'O':

			case 'U':

			case 'Y':
				break;

			case 'B':

			case 'F':

			case 'P':

			case 'V':
				if(*(rptr - 1) != '1') 
					*(rptr++) = '1';
				break;

			case 'C':

			case 'G':

			case 'J':

			case 'K':

			case 'Q':

			case 'S':

			case 'X':

			case 'Z':
				if(*(rptr - 1) != '2') 
					*(rptr++) = '2';
				break;

			case 'D':

			case 'T':
				if(*(rptr - 1) != '3') 
					*(rptr++) = '3';
				break;

			case 'L':
				if(*(rptr - 1) != '4')
					*(rptr++) = '4';
				break;

			case 'M':

			case 'N':
				if(*(rptr - 1) != '5') 
					*(rptr++) = '5';
				break;

			case 'R':
				if(*(rptr -1) != '6') 
					*(rptr++) = '6';
			default:
				break;
		}
	}

	for(; rptr < buffer.str + 4; rptr++)
		*rptr = '0';

	*(buffer.str + 4) = '\0';

	return buffer;
}

mybbstring fnc_pad(char *padchar, char *type, int newlength, mybbstring original)
	{
	int oldlength, offset = 0, x, ctr = 0;
	mybbstring new;

	oldlength = original.len; 

	new.str = SafeMalloc(newlength);
	new.len = newlength;

	for (x=0; x<newlength; x++)
		new.str[x] = '\0';

	if (oldlength > newlength) 
	{
		for (x=0; x<newlength; x++)
			new.str[x] = original.str[x];

		return new;
	}

	if (type[0] == 'L') 
	{
		for (x=oldlength; x<newlength; x++)
			new.str[x] = padchar[0];

	} else if (type[0] == 'C') 
	{
		offset = (newlength/2)-(oldlength/2);
		for (x=oldlength; x>=0; x--, ctr++)
			new.str[newlength-offset-ctr] = original.str[x];

		for (x=0; x<newlength-offset-oldlength; x++)
			new.str[x] = padchar[0];

		for (x=newlength-offset; x<=newlength-1; x++)
			new.str[x] = padchar[0];

	} else if (type[0] == 'R') 
	{
		for (x=oldlength-1; x>=0; x--, ctr++)
			new.str[newlength-1-ctr] = original.str[x];

		for (x=0; x<newlength-oldlength; x++)
			new.str[x] = padchar[0];
	}

	return new;
}

mybbstring fnc_ntd(mybbfloat date)
{
	int day = 1;  // whole part of date, day month and year
	float hour = 0; // fractional part, hour of day
	int x = 0;
	int monctr = 1, dayctr = 1, yearctr = 1;
	mybbstring out;

	out.str = SafeMalloc(11);
	out.len = 11;
	day = flt2int(date);
	hour = flt2dbl(date)-day;

	for (x=1; x<=day; x++) 
	{
		if (dayctr == 365) 
		{
			if ((yearctr % 4) == 0) 
			{
				if ((yearctr % 100) == 0) 
				{
					if ((yearctr % 400) == 0) 
						goto leapyear;
					else
						goto normalyear;
				}
leapyear:
				dayctr++;
			} else {
normalyear:
				dayctr = 1;
				yearctr++;
			}
		} else if (dayctr == 366) 
		{
			dayctr = 1;
			yearctr++;
		} else 
			dayctr++;
	}

	x = 0;
	 
	for (monctr = 0; monctr < 12; monctr++) 
	{
		if (monctr == 1) 
		{
			if (!((yearctr) % 4)) 
			{
				if (!((yearctr) % 100)) 
				{
					if (!((yearctr) % 400)) 
						x++;
				}
			}
		}

		if (dayctr - x <= envinfo.datestrings[monctr].numdays) 
			break;

		x += envinfo.datestrings[monctr].numdays;
	}

	sprintf(out.str, "%.2d-%.3s-%.4d", dayctr - x, envinfo.datestrings[monctr].name, yearctr);

	return out;
}

mybbstring fnc_ath(mybbstring in)
{
	int y = 0, x = 0;
	mybbstring out;

	out.str = SafeMalloc(in.len/2+1);
	*out.str = 0;

	for (y=0; y<in.len; y+=2) 
	{
		out.str[x] = myhtoi(in.str[y], in.str[y+1]);
		x++;
	}

	out.len = x;

	return out;
}

mybbstring fnc_hta(mybbstring c)
{
	char *strerg, **tmp2;
	int ctr, len = 0;
	mybbstring tmp;

	tmp2 = &c.str;
	len = c.len;

	if (!*tmp2 || len<1) 
		return c;

	strerg = SafeMalloc(len*2+1);
	tmp.len = len*2+1;
	tmp.str=strerg;

	for (ctr=0; ctr<len; ctr++) 
	{
		itoh((unsigned char)(*tmp2)[ctr],strerg);
		strerg+=2;
	}

	return tmp;
}

mybbstring fnc_ucs(mybbstring s)
{
	char *tmp;
	tmp = s.str;
	
	while(*tmp)
	{
		*tmp=toupper(*tmp++);
	}
	return s;
}

mybbstring fnc_lcs(mybbstring s)
{
	char *tmp;
	tmp = s.str;
	
	while(*tmp)
	{
		*tmp=tolower(*tmp++);
	}

	return s;
}

mybbstring fnc_pgm(program *pgm, byte linenum)
{
	mybbstring out, out2;
	statement *tempstmt = pgm->firststmt;
	byte i = 0, x = 0;

	do 
	{
		if (linenum == tempstmt->linenum) 
		{
			out.str = SafeMalloc(tempstmt->length*2);
			out.len = tempstmt->length * 2;
			sprintf(out.str, "%.2X%.2X", tempstmt->length, tempstmt->linenum);
            
			if (tempstmt->numlabels) 
			{
			}
			
			i += sprintf(out.str+i, "%X", tempstmt->opcode); 
			
			for (x=1; x<tempstmt->metapos; x++)
			{
				if (tempstmt->metalist[x].operation == FLOAT) 
				{
					if (!tempstmt->metalist[x].floatarg.exp) 
					{
						if (tempstmt->metalist[x].floatarg.mantisa.i <= 255)
							i += sprintf(out.str+i, "%X%.2lX", INTEGER, tempstmt->metalist[x].floatarg.mantisa.i);
						else
							i += sprintf(out.str+i, "%X%.4lX", LONGINTEGER, tempstmt->metalist[x].floatarg.mantisa.i);
					} else if (tempstmt->metalist[x].floatarg.exp == -2)
						i += sprintf(out.str+i, "%X%.4lX", FLOATIMPLIED, tempstmt->metalist[x].floatarg.mantisa.i);
					else 
						i += sprintf(out.str+i, "%X%.2X%X%.4lX", FLOAT, tempstmt->metalist[x].floatarg.exp, 0x0000, tempstmt->metalist[x].floatarg.mantisa.i);
				} else
					i += sprintf(out.str+i, "%X", tempstmt->metalist[x].operation);
				
				if (tempstmt->metalist[x].shortarg)
					i += sprintf(out.str+i, "%.4X", tempstmt->metalist[x].shortarg);
				
				if (tempstmt->metalist[x].intarg)
					i += sprintf(out.str+i, "%.2X", tempstmt->metalist[x].intarg);
				
				if (tempstmt->metalist[x].stringarg[0] != '\0')
				{
					strexecpush2(tempstmt->metalist[x].stringarg, tempstmt->metalist[x].intarg);
					out2 = fnc_hta(strexecpop());
					i += sprintf(out.str+i, "%s", out2.str);
				}
				
				sprintf(out.str+i, "%X", 0xF2);
			}

			goto finish;
		}
		tempstmt = tempstmt->nextstmt;
	} while (tempstmt != NULL);

	// if it reaches this point it didn't find a valid line
	out.len = 0;

	finish:

	return fnc_ath(out);
}

int fnc_tcb(int num)
{ 
	int x = 0;

	switch (num) 
	{
		case 0:

		case 1:

		case 2:
			break;

		case 3:
			break;

		case 4:
			x = envinfo.lastexec;
			break;

		case 5:
			x = envinfo.lasterrorline;
			break;

		case 6:
			x = envinfo.escline;
			break;

		case 7:
			x = envinfo.seterrline;
			break;

		case 8:
			break;

		case 9:
			break;

		case 13:
			x = envinfo.envlevel;  
			break;

		default:
			rterror(ERR_INTEGER_RANGE);
			break;
	}

	return x;
}

int fnc_err(statement *stmt, int start)
{
	int total = 0, x = 0, tmp = 0;

	total = stmt->metalist[start].intarg;

	for (x = 0; x < total; x++) 
	{
		tmp = flt2int(fltexecpop());
		if (tmp == envinfo.lasterror)
			break;

	}

	for (tmp = x; tmp < total; tmp++)
		fltexecpop();

	return total - x;
}

long fnc_rti(const char *str)
{
      int i, j, k;
      long retval = 0L;

      if (!str || '\0' == *str)
            return -1L;
      for (i = 0, k = -1; str[i]; ++i)
      {
            for (j = 0; j < 7; ++j)
            {
                  if (numerals[j].ch == toupper(str[i]))
                        break;
            }
            if (7 == j)
                  return -1L;
            if (k >= 0 && k < j)
            {
                  retval -= numerals[k].val * 2;
                  retval += numerals[j].val;
            }
            else  retval += numerals[j].val;
            k = j;
      }
      return retval;
}

