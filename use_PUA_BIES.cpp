#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "table.h"
#include "gaiji.h"
#define MAX_BYTE_PER_LINE 2048*3
#define MAX_WORD_PER_LINE	1024*2


unsigned short *unsigned_short_strstr(const unsigned short *haystack, size_t haystack_size, const unsigned short *needle, size_t needle_size)
{
   if (needle_size > haystack_size)
   {
      return NULL;
   }
   for (size_t i = 0; i <= haystack_size - needle_size; ++i)
   {
      size_t j;
      for (j = 0; j < needle_size; ++j)
      {
         if (haystack[i + j] != needle[j])
         {
            break;
         }
      }
      if (j == needle_size)
      {
         return (unsigned short *)&haystack[i]; // 找到匹配，返回匹配位置的指標
      }
   }
   return NULL; // 未找到匹配
}
/*
void replaceWord(unsigned short *article, size_t *articleLen, unsigned short *targetWord, size_t targetWordLen, unsigned short *newWord, size_t newWordLen)
{
   unsigned short *p = unsigned_short_strstr(article, *articleLen, targetWord, targetWordLen);

   while (p != NULL)
   {
      if (newWordLen > targetWordLen)
      {
         memmove(p + newWordLen, p + targetWordLen, (*articleLen - (p - article) - targetWordLen) * sizeof(unsigned short));
      }
      else if (newWordLen < targetWordLen)
      {
         memmove(p + newWordLen, p + targetWordLen, (*articleLen - (p - article) - targetWordLen) * sizeof(unsigned short));
      }
      memcpy(p, newWord, newWordLen * sizeof(unsigned short));

      // 更新文章長度
      *articleLen = *articleLen + newWordLen - targetWordLen;

      p = unsigned_short_strstr(p + newWordLen, *articleLen - (p - article) - newWordLen, targetWord, targetWordLen); // 搜索下一目標
   }
}
*/

void replaceWord(unsigned short *article, size_t *articleLen, unsigned short *targetWord, size_t targetWordLen, unsigned short *newWord, size_t newWordLen)
{
   unsigned short *p = unsigned_short_strstr(article, *articleLen, targetWord, targetWordLen);
   
   if (p != NULL)
   {
      if (newWordLen > targetWordLen)
      {
         memmove(p + newWordLen, p + targetWordLen, (*articleLen - (p - article) - targetWordLen) * sizeof(unsigned short));
      }
      else if (newWordLen < targetWordLen)
      {
         memmove(p + newWordLen, p + targetWordLen, (*articleLen - (p - article) - targetWordLen) * sizeof(unsigned short));
      }
      memcpy(p, newWord, newWordLen * sizeof(unsigned short));

      // 更新文章長度
      *articleLen = *articleLen + newWordLen - targetWordLen ;
      printf("replaced !!! \n");
      //p = unsigned_short_strstr(p + newWordLen, *articleLen - (p - article) - newWordLen, targetWord, targetWordLen); // 搜索下一目標
   }

}

int is_chinese_or_punctuation(unsigned short input)
{
   if ((input >= 0x4E00 && input <= 0x9FFF) || // 判斷輸入值是否屬於中文字符範圍（0x4E00-0x9FFF）或標點符號字符範圍（0x2000-0x206F）
       (input >= 0x2000 && input <= 0x206F) ||
       (input >= 0xFF01 && input <= 0xFF0F) ||
       (input >= 0xFF1A && input <= 0xFF20) ||
       (input >= 0xFF3B && input <= 0xFF40) ||
       (input >= 0xFF5B && input <= 0xFF65) ||
       (input >= 0x0020 && input <= 0x002F) ||
       (input >= 0x3000 && input <= 0x303F))
   {
      return 1; // 是中文或標點符號，返回1
   }
   else
   {
      return 0; // 不是中文或標點符號，返回0
   }
}


int targetEqual(unsigned short *arr1,const unsigned short *arr2, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (arr1[i] != arr2[i]) 
        {
            return 0; // 数组内容不相等
        }
    }
    return 1; // 数组内容相等
}


size_t replacePinyin(unsigned short word[], int no_char)
{
   size_t articleLen = (size_t)no_char;
   int TLsoo_table_length = sizeof(TLsoo_half_table) / sizeof(TLsoo_half_table[0]); // 計算TABLE結構體數組的長度
   int target_length = 0; // target中當前存儲的字節個數
   int max_target_length = 8;
   int previous_len;
   unsigned short* targetWord = (unsigned short *)malloc(max_target_length * sizeof(unsigned short));
   
   for (int i = 0; i < no_char; i++)
   {
      //printf("The current length: %ld \n", articleLen);
      
      unsigned short current_word = word[i];

      if (i == articleLen-1)
      {
          if ((is_chinese_or_punctuation(current_word) == 1 || target_length == max_target_length) && target_length != 0)
          {
             // 1. Search for TLsoo table
             for (int k = 0; k < TLsoo_table_length; k++)
             {
                int pinyin_size = 0;
                
                for (int n = 0; n < max_target_length; n++)
                {
                   if (TLsoo_half_table[k].pinyin[n] != 0x0000)
                   {
                      pinyin_size++;
                   }
                }
                
                unsigned short newWord = TLsoo_half_table[k].newunicode;
                                    
                if ( targetEqual(targetWord, TLsoo_half_table[k].pinyin, pinyin_size) == 1 )
                {
                    previous_len = articleLen;
                    replaceWord(word, &articleLen, targetWord, target_length, &newWord, 1);
                    
                    printf("The replaced article: ");
                    for (int a = 0; a < articleLen; a++)
                    {
                        printf("0x%04X ",word[a]);
                    }
                    printf("\n");
                      
                    i = i - ( previous_len - articleLen );
                    target_length = 0;
                }
             }
             target_length = 0; // table 搜尋結束，未找到目標字串，目標長度歸零
          }                      
      }
      else
      {
          if (is_chinese_or_punctuation(current_word) == 0 && target_length < max_target_length) // 當前unicode不是中文或者標點符號
          {
             if ((current_word >= 0x00C0 && current_word <= 0x024F) || 
                 (current_word >= 0x0300 && current_word <= 0x036F) || 
                 (current_word >= 0xFF21 && current_word <= 0xFF3A) || 
                 (current_word >= 0xFF10 && current_word <= 0xFF19) || 
                 (current_word >= 0xFF41 && current_word <= 0xFF5A) ||
                 (current_word >= 0x0030 && current_word <= 0x0039) ||
                 (current_word >= 0x0041 && current_word <= 0x005A) ||
                 (current_word >= 0x0061 && current_word <= 0x007A)
                 ) // 當前unicode是全形的英文或者數字
             {
                targetWord[target_length] = current_word;
                
                //printf("The target word: 0x%04X; ",targetWord[target_length]);
                
                target_length++;
             }

          }
          printf("0x%04X index: %d length: %d \n",  word[i] , i, target_length);
          
          if ((is_chinese_or_punctuation(current_word) == 1 || target_length == max_target_length) && target_length != 0)
          {
             
             // 1. Search for TLsoo table
             printf("Come in search table \n");
             for (int k = 0; k < TLsoo_table_length; k++)
             {
                int pinyin_size = 0;
                
                for (int n = 0; n < max_target_length; n++)
                {
                   if (TLsoo_half_table[k].pinyin[n] != 0x0000)
                   {
                      pinyin_size++;
                   }
                }
                
                unsigned short newWord = TLsoo_half_table[k].newunicode;
                                    
                if ( targetEqual(targetWord, TLsoo_half_table[k].pinyin, pinyin_size) == 1)
                {
                    previous_len = articleLen;
                    replaceWord(word, &articleLen, targetWord, target_length, &newWord, 1);
                    
                    printf("The replaced article: ");
                    for (int a = 0; a < articleLen; a++)
                    {
                        printf("0x%04X (%d) ",word[a], a);
                    }
                    printf("\n");
                    
                    i = i - ( previous_len - articleLen );
                    target_length = 0;
                }
             }
             target_length = 0; // table 搜尋結束，未找到目標字串，目標長度歸零
          }      
       }
   }
   free(targetWord);
   return articleLen;
}

void UTF_8ToUnicode(unsigned char *pOut, unsigned char *pText, int *len)
{
   unsigned char *uchar = pOut;
   unsigned char *p_In = pText;

   int table_length = sizeof(gaiji_table) / sizeof(gaiji_table[0]); // 計算GAIJI的結構體數組的長度

   *len = 0;

   while (1)
   {
      /* most frequently case */
      if ((p_In[0] & 0xf0) == 0xe0 &&
          (p_In[1] & 0xc0) == 0x80 && 
          (p_In[2] & 0xc0) == 0x80)
      {
         uchar[1] = ((p_In[0] & 0x0F) << 4) + ((p_In[1] >> 2) & 0x0F);
         uchar[0] = ((p_In[1] & 0x03) << 6) + (p_In[2] & 0x3F);
         *len += 3;
         break;
      }
      /* 2nd frequently case */
      else if ((p_In[0] & 0x80) == 0x00)
      {
         uchar[1] = 0;
         uchar[0] = p_In[0];
         *len += 1;
         break;
      }
      else if ((p_In[0] & 0xe0) == 0xc0 && (p_In[1] & 0xc0) == 0x80)
      {
         uchar[1] = ((p_In[0] & 0x1F) >> 2);
         uchar[0] = ((p_In[0] & 0x03) << 6) + (p_In[1] & 0x3F);
         *len += 2;
         break;
      }
      else if ((p_In[0] & 0xf8) == 0xf0 && 
               (p_In[1] & 0xc0) == 0x80 && 
               (p_In[2] & 0xc0) == 0x80 && 
               (p_In[3] & 0xc0) == 0x80)
      {
         printf("4-byte UTF-16 character : 0X%02X 0X%02X 0X%02X 0X%02X, skip it\n", p_In[0], p_In[1], p_In[2], p_In[3]);
         // new added for outer_word <begin>
         for (int i = 0; i < table_length; i++)
         {
            if ((p_In[0] == gaiji_table[i].utf8[0]) &&
                (p_In[1] == gaiji_table[i].utf8[1]) && 
                (p_In[2] == gaiji_table[i].utf8[2]) && 
                (p_In[3] == gaiji_table[i].utf8[3]))
            {
               uchar[1] = ((gaiji_table[i].innercode >> 8) & 0xFF);
               uchar[0] = (gaiji_table[i].innercode & 0xFF);
            }
         }
         // new added for outer_word <end>
         p_In += 4;
         *len += 4;
         break;
      }
      else
      {
         printf("non-UTF-8 error : 0X%02X\n, skip one byte\n", p_In[0]);
         p_In += 1;
         *len += 1;
      }
   }

   return;
}

int To_Unicode(unsigned char *org_line, unsigned short *uni_line, int *no_char)
{
   int j = 0;
   int len;
   unsigned char *p;
   unsigned char *limit;
   unsigned char out[2];
   short code;

   p = org_line;

   limit = org_line + MAX_BYTE_PER_LINE;

   while (*p != '\0')
   {
      UTF_8ToUnicode(out, p, &len);

      if (p >= limit)
      {
         return 1;
      }

      if (j >= MAX_BYTE_PER_LINE)
      {
         return 2;
      }

      if ((out[1] == 0x00 && (out[0] == 0x20 || out[0] == 0x09 /*TAB*/ || out[0] == 0xa0)) || // some special blank
          (out[1] == 0x20 && (out[0] == 0x07 || out[0] == 0x2f || out[0] == 0x60)))
      {
         out[1] = 0x30;
         out[0] = 0x00;
      }
      
      if ((out[1] == 0x30 && out[0] == 0x00) && (j != 0 && uni_line[j - 1] == 0x3000))
      // double space change into space
      {
         p += len;
         continue;
      }
      /*
      // 半形轉全形
      if (out[1] == 0x00 && out[0] > 0x20 && out[0] <= 0x7E)
      {
         out[1] = 0xFF;
         out[0] -= 0x20;
      }
      */
      code = ((unsigned short)out[1]) << 8 | (unsigned short)out[0];

      uni_line[j] = code;
      j++;
      p += len;
   }
   uni_line[j] = 0;
   *no_char = j;

   return 0;
}

void unicodeToUtf8(unsigned short unicode, unsigned char *s)
{
   int table_length = sizeof(gaiji_table) / sizeof(gaiji_table[0]); // 計算GAIJI的結構體數組的長度
   if ((unicode >= 0x0800) && (unicode <= 0xF251))
   {
      *s++ = 0xe0 | (char)((unicode >> 12) & 0x0f);
      *s++ = 0x80 | (char)((unicode >> 6) & 0x3f);
      *s++ = 0x80 | (char)(unicode & 0x3f);
      *s++ = 0;
   }

   // new added (solving outer word) <begin>
   else if ((unicode >= 0xF252) && (unicode <= 0xF2BD))
   {
      for (int i = 0; i < table_length; i++)
         if (unicode == gaiji_table[i].innercode)
         {
            for (int j = 0; j < 4; j++)
            {
               *s++ = gaiji_table[i].utf8[j];
            }
         }
   }
   // new added (solving outer word) <end>

   else if ((unicode >= 0xF2BE) && (unicode <= 0xFFFF))
   {
      *s++ = 0xe0 | (char)((unicode >> 12) & 0x0f);
      *s++ = 0x80 | (char)((unicode >> 6) & 0x3f);
      *s++ = 0x80 | (char)(unicode & 0x3f);
      *s++ = 0;
   }

   else if ((unicode >= 0x0080) && (unicode <= 0x07FF))
   {
      *s++ = 0xC0 | (char)((unicode >> 6) & 0x1f);
      *s++ = 0x80 | (char)(unicode & 0x3f);
      *s++ = 0;
   }

   else if (unicode < 0x0080)
   {
      *s++ = unicode;
      *s++ = 0;
   }
   return;
}
/*
void To_UTF8(unsigned char *out_word, unsigned short *word, int word_len)
{
   // void unicodeToUtf8(unsigned short unicode, unsigned char *s)  // 1-byte unicode will be happened for [Neu]
   unsigned char s[4];
   int max_pinyin_len = 8;
   int table_length = sizeof(TLsoo_table) / sizeof(TLsoo_table[0]); // 計算TABLE結構體數組的長度

   if ((word[0] >= 0xE000) && (word[0] <= 0xF251))
   {
      for (int x = 0; x < table_length; x++)
      {
         if (word[0] == TLsoo_table[x].newunicode)
         {
            int pinyin_size = 0;

            for (int y = 0; y < max_pinyin_len; y++)
            {
               if (TLsoo_table[x].pinyin[y] != 0x0000)
               {
                  pinyin_size++;
               }
            }

            for (int z = 0; z < pinyin_size; z++)
            {
               if (z == 0)
               {
                  unicodeToUtf8(TLsoo_table[x].pinyin[0], s);
                  strcpy((char *)out_word, (char *)s);
               }
               else
               {
                  unicodeToUtf8(TLsoo_table[x].pinyin[z], s);
                  strcat((char *)out_word, (char *)s);
               }
            }
         }
      }
   }
   else
   {
      unicodeToUtf8(word[0], s);
      strcpy((char *)out_word, (char *)s);
   }

   for (int i = 1; i < word_len; i++)
   {
      if ((word[i] >= 0xE000) && (word[i] <= 0xF251))
      {
         for (int j = 0; j < table_length; j++)
         {
            if (word[i] == TLsoo_table[j].newunicode)
            {
               int ori_size = 0;

               for (int n = 0; n < max_pinyin_len; n++)
               {
                  if (TLsoo_table[j].pinyin[n] != 0x0000)
                  {
                     ori_size++;
                  }
               }

               for (int k = 0; k < ori_size; k++)
               {
                  unicodeToUtf8(TLsoo_table[j].pinyin[k], s);
                  strcat((char *)out_word, (char *)s);
               }
            }
         }
      }
      else
      {
         unicodeToUtf8(word[i], s);
         strcat((char *)out_word, (char *)s);
      }
   }
   return;
}
*/

void To_UTF8(unsigned char *out_word, unsigned short *word, int word_len)
{
//void unicodeToUtf8(unsigned short unicode, unsigned char *s)  // 1-byte unicode will be happened for [Neu]
         int j;
         unsigned char s[4];

         unicodeToUtf8(word[0], s);
         strcpy((char *)out_word, (char *)s);
             for(j=1; j<word_len; j++)
         {
            unicodeToUtf8(word[j], s);
            strcat((char *)out_word, (char *)s);
         }

         return;
}

int main() 
{
    FILE *input_file, *output_file;
    char filename[] = "test.txt";
    char output_filename[] = "output.txt";
    char utf8_str[MAX_WORD_PER_LINE];
    unsigned char UTF_word[MAX_WORD_PER_LINE];
    unsigned char r_word[MAX_WORD_PER_LINE]; // 用來儲存還原的utf8字串
    unsigned short word[MAX_BYTE_PER_LINE / 3]; // 用來儲存unicode
    size_t articleLen;
    size_t dataSize = 0;
    int no_char;
    
    input_file = fopen(filename, "r");
    if (input_file == NULL) {
        printf("Error opening input file\n");
        return 1;
    }

    output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        printf("Error opening output file\n");
        fclose(input_file);
        return 1;
    }

    while (fgets(utf8_str, sizeof(utf8_str), input_file) != NULL)
    {
        for (int i = 0; i < MAX_WORD_PER_LINE; i++)
        {
            UTF_word[i] = (unsigned char)utf8_str[i];
        }
        // 將utf8字串轉為unicode sequence存在word裡面
        To_Unicode(UTF_word, word, &no_char);
        
        printf("The original unicode atricle: ");
        for (int k = 0; k < no_char; k++)
        {
            printf("0x%04X ", UTF_word[k]);
        }
        printf("\n");
        
        articleLen = replacePinyin(word, no_char);
        /*
        printf("The new atricle: ");
        for(int j = 0; j < articleLen; j++){
             printf("0x%04X ", word[j]);
        }
        printf("\n");
        */
        To_UTF8(r_word, word, articleLen);
                 
        while (dataSize < MAX_WORD_PER_LINE)
        {
            if (r_word[dataSize - 1] == 0x000D && r_word[dataSize] == 0x000A)
            {
                dataSize++;
                break;
            }
            else if (r_word[dataSize] == 0x0000)
            {
                break;
            }
            dataSize++;
        }
        
        printf("data size: %ld \n", dataSize);
        printf("The utf8 article: ");
        for(int k = 0; k < dataSize; k++)
        {
          printf("0x%04X ", r_word[k]);
        }
        printf("\n");   
        
        fwrite(r_word, 1, dataSize, output_file);
        dataSize = 0;      
    }
    fclose(input_file);
    fclose(output_file);
    return 0;
}