/* Module FORMDATE.C
   This file contains functions for the date-form. Different countries use
   different conventions for date, and some use more than one. The date may
   differ in the order between the 3 components, the delimiter to use and the
   length of the year component (4 or 2 digits). The form to use will be
   loaded from the current config-file when needed (default setting is the
   Swedish, logical, with descending order dashes and two-digits. */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include <allegro.h>
#include "cgui.h"
#include "cgui/formdate.h"
#include "cgui/mem.h"
#include "labldata.h"
#include "formdate.ht"

static char const *const *current_format_letters;
static char const *const *short_months;
static char const *const *long_months;
static char const *const *short_days;
static char const *const *long_days;
static char *current_date_format;

/* Inititalises the date format to the default one found in config file */
static void InitDateForm(void)
{
   static int date_form_loaded;
   int nr;
   char *dateform;

   if (date_form_loaded)
      return;
   current_format_letters=LoadCompiledTexts(cgui_labels+CGUI_CGUILABL,"formatletters",&nr);
   assert(nr==SIZE_formatletters);
   short_months=LoadCompiledTexts(cgui_labels+CGUI_CGUILABL,"shortmonths",&nr);
   assert(nr==SIZE_shortmonths);
   long_months=LoadCompiledTexts(cgui_labels+CGUI_CGUILABL,"longmonths",&nr);
   assert(nr==SIZE_longmonths);
   short_days=LoadCompiledTexts(cgui_labels+CGUI_CGUILABL,"shortdays",&nr);
   assert(nr==SIZE_shortdays);
   long_days=LoadCompiledTexts(cgui_labels+CGUI_CGUILABL,"longdays",&nr);
   assert(nr==SIZE_longdays);
   dateform=GetMem(char,12);
   sprintf(dateform,"%c%c-%c%c%c-%c%c%c%c",*current_format_letters[LETTER_DAY],*current_format_letters[LETTER_DAY],
      *current_format_letters[LETTER_MONTH],*current_format_letters[LETTER_MONTH],*current_format_letters[LETTER_MONTH],
      *current_format_letters[LETTER_YEAR],*current_format_letters[LETTER_YEAR],
      *current_format_letters[LETTER_YEAR],*current_format_letters[LETTER_YEAR]);
   current_date_format=dateform;
   date_form_loaded = 1;
}
static int CountFormat(const char **fp,char c)
{
   int i;
   for (i = 0; **fp == c; i++, (*fp)++)
      ;
   return i;

}
extern void SetDateForm(const char *format)
{
   Release(current_date_format);
   current_date_format=MkString(format);
}

extern int CheckDate(struct tm *d, const char *format)
{
   static int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
   int use_d = 0, use_m = 0, use_y = 0;

   if (format == NULL || *format == 0)
      format = current_date_format;
   for (; *format; format++) {
      if (*format == *current_format_letters[LETTER_YEAR])
         use_y = 1;
      if (*format == *current_format_letters[LETTER_MONTH])
         use_m = 1;
      if (*format == *current_format_letters[LETTER_DAY])
         use_d = 1;
   }
   if (use_d && d->tm_mday < 1)
      return 0;
   if (use_m && (d->tm_mon < 1 || d->tm_mon > 12))
      return 0;
   if (!use_m || !use_d || !use_y)
      return 1;
   /* the last check requires all components to be present */
   return d->tm_mday <=
       (days[(int) d->tm_mon - 1] +
        ((d->tm_mon == 2 && (d->tm_year % 4) == 0) && d->tm_year != 1900));
}

static int StringInd(const char *const *strings,int n,const char **in)
{
   int *miss,cmiss;
   int i,tkn;
   miss=GetMem0(int,n);

   for(tkn=0,cmiss=0;cmiss<n;tkn++)
      for(i=0;i<n;i++)
      {
         if(!miss[i])
         {
            if(strings[i][tkn]=='\0')
            {
               Release(miss);
               *in+=tkn;
               return i;
            }

            if(strings[i][tkn]!=(*in)[tkn])
            {
               miss[i]=1;
               cmiss++;
            }
         }
      }
   Release(miss);
   return -1;
}

extern int String2Date(struct tm *d, const char *s, const char *format)
{
   int day_done = 0, year_done = 0, month_done = 0,hour_done=0,minute_done=0,second_done=0;
   time_t now;
   int thisYear,read;
   InitDateForm();
   d->tm_year = d->tm_mon = d->tm_mday = d->tm_hour = d->tm_min = d->tm_sec=0;
   d->tm_isdst = 0;
   if (format == NULL || *format == 0)
      format = current_date_format;
   while (isspace(*s))
      s++;
   while (*format == '\t' || *format == ' ')
      format++;
   while (*s && *format) {
      if (*format == *current_format_letters[LETTER_YEAR]) {
         if (year_done)
            return 0;
         year_done = 1;
         switch(CountFormat(&format,*format))
       {
          case 2:
             now=time(NULL);
             thisYear=localtime(&now)->tm_year;
             sscanf(s,"%2d%n",&d->tm_year,&read);
             s+=read;
             d->tm_year+=-(d->tm_year+70-thisYear-100)/100*100;
             break;
          case 4:
             sscanf(s,"%4d%n",&d->tm_year,&read);
             s+=read;
             d->tm_year-=1900;
             break;
          default:
             return 0;
             break;
       }
      } else if (*format == *current_format_letters[LETTER_MONTH]) {
         if (month_done)
            return 0;
         month_done = 1;
         switch(CountFormat(&format,*format))
       {
         case 1:
         case 2:
            sscanf(s,"%2d%n",&d->tm_mon,&read);
            s+=read;
            d->tm_mon--;
            break;

         case 3:
            d->tm_mon=StringInd(short_months,SIZE_shortmonths,&s);
            if(d->tm_mon==-1)
               return 0;
            break;
         case 4:
            d->tm_mon=StringInd(long_months,SIZE_longmonths,&s);
            if(d->tm_mon==-1)
               return 0;
            break;
         default:
            return 0;
            break;
       }

      } else if (*format == *current_format_letters[LETTER_DAY]) {
         if (day_done)
            return 0;
         day_done = 1;
       switch(CountFormat(&format,*format))
       {
         case 1:
         case 2:
            sscanf(s,"%2d%n",&d->tm_mday,&read);
            s+=read;
            break;
         case 3:
            StringInd(short_days,SIZE_shortdays,&s);
            day_done=0;
            break;
         case 4:
            StringInd(long_days,SIZE_longdays,&s);
            day_done=0;
            break;
         default:
            return 0;
            break;
       }
      } else if (*format == *current_format_letters[LETTER_HOUR]) {
         if (hour_done)
            return 0;
         hour_done = 1;
       switch(CountFormat(&format,*format))
       {
         case 1:
         case 2:
            sscanf(s,"%2d%n",&d->tm_hour,&read);
            s+=read;
            break;
         default:
            return 0;
            break;
       }
      } else if (*format == *current_format_letters[LETTER_MINUTE]) {
         if (minute_done)
            return 0;
         minute_done = 1;
       switch(CountFormat(&format,*format))
       {
         case 1:
         case 2:
            sscanf(s,"%2d%n",&d->tm_min,&read);
            s+=read;
            break;
         default:
            return 0;
            break;
       }
      } else if (*format == *current_format_letters[LETTER_SECOND]) {
         if (second_done)
            return 0;
         second_done = 1;
       switch(CountFormat(&format,*format))
       {
         case 1:
         case 2:
            sscanf(s,"%2d%n",&d->tm_sec,&read);
            s+=read;
            break;
         default:
            return 0;
            break;
       }
     } else  {      /* a delimiter ?*/
         if(*format=='\'')
       {
          format++;
          while(*format!='\'')
          {
             format++;
             s++;
          }
          format++;
          if(*s=='\0')
             return 0;
       }
       else
       {
          while(*s&&*format!=*s)
             s++;
          format++;
          if(*s=='\0')
            return 0;
          s++;
       }
      }
      while (isspace(*s))
         s++;
      while (*format == '\t' || *format == ' ')
         format++;
   }
   if(day_done && month_done && year_done)
   {
      mktime(d);
      return 1;
   }
   return 0;
}

extern int Date2String(struct tm *d, char *s, const char *format)
{
   int delim = 0;

   if (!d->tm_year && !d->tm_mon && !d->tm_mday) {
      *s = 0;
      return 0;
   }
   InitDateForm();
   if (format == NULL || *format == 0)
      format = current_date_format;
   while (*format == '\t' || *format == ' ')
      format++;
   while (*format) {
      if (*format == *current_format_letters[LETTER_YEAR]) {
         delim = 0;
         switch(CountFormat(&format,*current_format_letters[LETTER_YEAR]))
       {
       case 1:
          s+=sprintf(s,"%d",d->tm_year%100);
          break;
       case 2:
          s+=sprintf(s,"%02d",d->tm_year%100);
          break;
       case 4:
          s+=sprintf(s,"%04d",d->tm_year+1900);
          break;
       default:
          return 0;
          break;
       }
      } else if (*format == *current_format_letters[LETTER_MONTH]) {
         delim = 0;
         switch(CountFormat(&format,*current_format_letters[LETTER_MONTH]))
       {
       case 1:
          s+=sprintf(s,"%d",d->tm_mon+1);
          break;
       case 2:
          s+=sprintf(s,"%02d",d->tm_mon+1);
          break;
       case 3:
          s+=sprintf(s,"%s",short_months[d->tm_mon]);
          break;
       case 4:
          s+=sprintf(s,"%s",long_months[d->tm_mon]);
          break;
       default:
          return 0;
          break;
       }
      } else if (*format == *current_format_letters[LETTER_DAY]) {
         delim = 0;
       switch(CountFormat(&format,*current_format_letters[LETTER_DAY]))
       {
       case 1:
          s+=sprintf(s,"%d",d->tm_mday);
          break;
       case 2:
          s+=sprintf(s,"%02d",d->tm_mday);
          break;
       case 3:
          s+=sprintf(s,"%s",short_days[d->tm_wday]);
          break;
       case 4:
          s+=sprintf(s,"%s",long_days[d->tm_wday]);
          break;
       default:
          return 0;
          break;
       }
     } else if (*format == *current_format_letters[LETTER_HOUR]) {
        delim=0;
        switch(CountFormat(&format,*format))
        {
        case 1:
           s+=sprintf(s,"%d",d->tm_hour);
           break;
        case 2:
           s+=sprintf(s,"%02d",d->tm_hour);
           break;
        }
     } else if (*format == *current_format_letters[LETTER_MINUTE]) {
        delim=0;
        switch(CountFormat(&format,*format))
        {
        case 1:
           s+=sprintf(s,"%d",d->tm_min);
           break;
        case 2:
           s+=sprintf(s,"%02d",d->tm_min);
           break;
        }
     } else if (*format == *current_format_letters[LETTER_SECOND]) {
        delim=0;
        switch(CountFormat(&format,*format))
        {
        case 1:
           s+=sprintf(s,"%d",d->tm_sec);
           break;
        case 2:
           s+=sprintf(s,"%02d",d->tm_sec);
           break;
        }

     } else {
         /* this is to avoid multiple delimiters in user format string that
            might damage the string */
         if (!delim)
       {
          if(*format=='\'')
          {
             format++;
             while(*format!='\'')
                *s++=*format++;
          }
          else
            *s++ = *format;
       }
         delim = 1;
         format++;
      }
   }
   *s = 0;
   return 1;
}
