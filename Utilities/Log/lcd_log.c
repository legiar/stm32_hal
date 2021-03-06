#include  <stdio.h>
#include  "lcd_log.h"


/* Define the display window settings */
#define     YWINDOW_MIN         4

LCD_LOG_line LCD_CacheBuffer [LCD_CACHE_DEPTH]; 
uint32_t LCD_LineColor;
uint16_t LCD_CacheBuffer_xptr;
uint16_t LCD_CacheBuffer_yptr_top;
uint16_t LCD_CacheBuffer_yptr_bottom;

uint16_t LCD_CacheBuffer_yptr_top_bak;
uint16_t LCD_CacheBuffer_yptr_bottom_bak;

FunctionalState LCD_CacheBuffer_yptr_invert;
FunctionalState LCD_ScrollActive;
FunctionalState LCD_Lock;
FunctionalState LCD_Scrolled;
uint16_t LCD_ScrollBackStep;

void LCD_LOG_Init ( void)
{
  // Deinit LCD cache
  LCD_LOG_DeInit();
  
  // Clear the LCD
  LCD_Clear(LCD_LOG_BACKGROUND_COLOR);  
}

void LCD_LOG_DeInit(void)
{
  LCD_LineColor = LCD_LOG_TEXT_COLOR;
  LCD_CacheBuffer_xptr = 0;
  LCD_CacheBuffer_yptr_top = 0;
  LCD_CacheBuffer_yptr_bottom = 0;
  
  LCD_CacheBuffer_yptr_top_bak = 0;
  LCD_CacheBuffer_yptr_bottom_bak = 0;
  
  LCD_CacheBuffer_yptr_invert= ENABLE;
  LCD_ScrollActive = DISABLE;
  LCD_Lock = DISABLE;
  LCD_Scrolled = DISABLE;
  LCD_ScrollBackStep = 0;
}

void LCD_LOG_SetHeader (uint8_t *header)
{
  /* Set the LCD Font */
  LCD_SetFont (&LCD_LOG_HEADER_FONT);

  LCD_SetTextColor(LCD_LOG_SOLID_BACKGROUND_COLOR);
  LCD_FillRect(0, 0, LCD_GetXSize(), LCD_LOG_HEADER_FONT.Height * 3);
  
  /* Set the LCD Text Color */
  LCD_SetTextColor(LCD_LOG_SOLID_TEXT_COLOR);
  LCD_SetBackColor(LCD_LOG_SOLID_BACKGROUND_COLOR);

  LCD_DisplayStringAt(0, LCD_LOG_HEADER_FONT.Height, header, CENTER_MODE);

  LCD_SetBackColor(LCD_LOG_BACKGROUND_COLOR);
  LCD_SetTextColor(LCD_LOG_TEXT_COLOR);
  LCD_SetFont (&LCD_LOG_TEXT_FONT);
}

/**
  * @brief  Display the application footer on the LCD screen 
  * @param  footer: pointer to the string to be displayed
  * @retval None
  */
void LCD_LOG_SetFooter(uint8_t *footer)
{
  /* Set the LCD Font */
  LCD_SetFont (&LCD_LOG_FOOTER_FONT);

  LCD_SetTextColor(LCD_LOG_SOLID_BACKGROUND_COLOR);
  LCD_FillRect(0, LCD_GetYSize() - LCD_LOG_FOOTER_FONT.Height - 4, LCD_GetXSize(), LCD_LOG_FOOTER_FONT.Height + 4);
  
  /* Set the LCD Text Color */
  LCD_SetTextColor(LCD_LOG_SOLID_TEXT_COLOR);
  LCD_SetBackColor(LCD_LOG_SOLID_BACKGROUND_COLOR);

  LCD_DisplayStringAt(0, LCD_GetYSize() - LCD_LOG_FOOTER_FONT.Height, footer, CENTER_MODE);

  LCD_SetBackColor(LCD_LOG_BACKGROUND_COLOR);
  LCD_SetTextColor(LCD_LOG_TEXT_COLOR);
  LCD_SetFont (&LCD_LOG_TEXT_FONT);
}

void LCD_LOG_ClearTextZone(void)
{
  uint8_t i=0;
  
  for (i= 0 ; i < YWINDOW_SIZE; i++)
  {
    LCD_ClearStringLine(i + YWINDOW_MIN);
  }
  
  LCD_LOG_DeInit();
}

LCD_LOG_PUTCHAR
{
  
  sFONT *cFont = LCD_GetFont();
  uint32_t idx;
  
  if(LCD_Lock == DISABLE)
  {
    if(LCD_ScrollActive == ENABLE)
    {
      LCD_CacheBuffer_yptr_bottom = LCD_CacheBuffer_yptr_bottom_bak;
      LCD_CacheBuffer_yptr_top    = LCD_CacheBuffer_yptr_top_bak;
      LCD_ScrollActive = DISABLE;
      LCD_Scrolled = DISABLE;
      LCD_ScrollBackStep = 0;
      
    }
    
    if(( LCD_CacheBuffer_xptr < (LCD_GetXSize()) /cFont->Width ) &&  ( ch != '\n'))
    {
      LCD_CacheBuffer[LCD_CacheBuffer_yptr_bottom].line[LCD_CacheBuffer_xptr++] = (uint16_t)ch;
    }   
    else 
    {
      if(LCD_CacheBuffer_yptr_top >= LCD_CacheBuffer_yptr_bottom)
      {
        
        if(LCD_CacheBuffer_yptr_invert == DISABLE)
        {
          LCD_CacheBuffer_yptr_top++;
          
          if(LCD_CacheBuffer_yptr_top == LCD_CACHE_DEPTH)
          {
            LCD_CacheBuffer_yptr_top = 0;  
          }
        }
        else
        {
          LCD_CacheBuffer_yptr_invert= DISABLE;
        }
      }
      
      for(idx = LCD_CacheBuffer_xptr ; idx < (LCD_GetXSize()) /cFont->Width; idx++)
      {
        LCD_CacheBuffer[LCD_CacheBuffer_yptr_bottom].line[LCD_CacheBuffer_xptr++] = ' ';
      }   
      LCD_CacheBuffer[LCD_CacheBuffer_yptr_bottom].color = LCD_LineColor;  
      
      LCD_CacheBuffer_xptr = 0;
      
      LCD_LOG_UpdateDisplay (); 
      
      LCD_CacheBuffer_yptr_bottom ++; 
      
      if (LCD_CacheBuffer_yptr_bottom == LCD_CACHE_DEPTH) 
      {
        LCD_CacheBuffer_yptr_bottom = 0;
        LCD_CacheBuffer_yptr_top = 1;    
        LCD_CacheBuffer_yptr_invert = ENABLE;
      }
      
      if( ch != '\n')
      {
        LCD_CacheBuffer[LCD_CacheBuffer_yptr_bottom].line[LCD_CacheBuffer_xptr++] = (uint16_t)ch;
      }
      
    }
  }
  return ch;
}
  
/**
  * @brief  Update the text area display
  * @param  None
  * @retval None
  */
void LCD_LOG_UpdateDisplay (void)
{
  uint8_t cnt = 0 ;
  uint16_t length = 0 ;
  uint16_t ptr = 0, index = 0;
  
  if((LCD_CacheBuffer_yptr_bottom  < (YWINDOW_SIZE -1)) && 
     (LCD_CacheBuffer_yptr_bottom  >= LCD_CacheBuffer_yptr_top))
  {
    LCD_SetTextColor(LCD_CacheBuffer[cnt + LCD_CacheBuffer_yptr_bottom].color);
    LCD_DisplayStringAtLine ((YWINDOW_MIN + LCD_CacheBuffer_yptr_bottom),
                           (uint8_t *)(LCD_CacheBuffer[cnt + LCD_CacheBuffer_yptr_bottom].line));
  }
  else
  {
    
    if(LCD_CacheBuffer_yptr_bottom < LCD_CacheBuffer_yptr_top)
    {
      /* Virtual length for rolling */
      length = LCD_CACHE_DEPTH + LCD_CacheBuffer_yptr_bottom ;
    }
    else
    {
      length = LCD_CacheBuffer_yptr_bottom;
    }
    
    ptr = length - YWINDOW_SIZE + 1;
    
    for  (cnt = 0 ; cnt < YWINDOW_SIZE ; cnt ++)
    {
      
      index = (cnt + ptr )% LCD_CACHE_DEPTH ;
      
      LCD_SetTextColor(LCD_CacheBuffer[index].color);
      LCD_DisplayStringAtLine ((cnt + YWINDOW_MIN), 
                             (uint8_t *)(LCD_CacheBuffer[index].line));
      
    }
  }
  
}

#if( LCD_SCROLL_ENABLED == 1)
/**
  * @brief  Display previous text frame
  * @param  None
  * @retval Status
  */
ErrorStatus LCD_LOG_ScrollBack(void)
{
    
  if(LCD_ScrollActive == DISABLE)
  {
    
    LCD_CacheBuffer_yptr_bottom_bak = LCD_CacheBuffer_yptr_bottom;
    LCD_CacheBuffer_yptr_top_bak    = LCD_CacheBuffer_yptr_top;
    
    
    if(LCD_CacheBuffer_yptr_bottom > LCD_CacheBuffer_yptr_top) 
    {
      
      if ((LCD_CacheBuffer_yptr_bottom - LCD_CacheBuffer_yptr_top) <=  YWINDOW_SIZE)
      {
        LCD_Lock = DISABLE;
        return ERROR;
      }
    }
    LCD_ScrollActive = ENABLE;
    
    if((LCD_CacheBuffer_yptr_bottom  > LCD_CacheBuffer_yptr_top)&&
       (LCD_Scrolled == DISABLE ))
    {
      LCD_CacheBuffer_yptr_bottom--;
      LCD_Scrolled = ENABLE;
    }
    
  }
  
  if(LCD_ScrollActive == ENABLE)
  {
    LCD_Lock = ENABLE;
    
    if(LCD_CacheBuffer_yptr_bottom > LCD_CacheBuffer_yptr_top) 
    {
      
      if((LCD_CacheBuffer_yptr_bottom  - LCD_CacheBuffer_yptr_top) <  YWINDOW_SIZE )
      {
        LCD_Lock = DISABLE;
        return ERROR;
      }
      
      LCD_CacheBuffer_yptr_bottom --;
    }
    else if(LCD_CacheBuffer_yptr_bottom <= LCD_CacheBuffer_yptr_top)
    {
      
      if((LCD_CACHE_DEPTH  - LCD_CacheBuffer_yptr_top + LCD_CacheBuffer_yptr_bottom) < YWINDOW_SIZE)
      {
        LCD_Lock = DISABLE;
        return ERROR;
      }
      LCD_CacheBuffer_yptr_bottom --;
      
      if(LCD_CacheBuffer_yptr_bottom == 0xFFFF)
      {
        LCD_CacheBuffer_yptr_bottom = LCD_CACHE_DEPTH - 2;
      }
    }
    LCD_ScrollBackStep++;
    LCD_LOG_UpdateDisplay();
    LCD_Lock = DISABLE;
  }
  return SUCCESS;
}

/**
  * @brief  Display next text frame
  * @param  None
  * @retval Status
  */
ErrorStatus LCD_LOG_ScrollForward(void)
{
  
  if(LCD_ScrollBackStep != 0)
  {
    if(LCD_ScrollActive == DISABLE)
    {
      
      LCD_CacheBuffer_yptr_bottom_bak = LCD_CacheBuffer_yptr_bottom;
      LCD_CacheBuffer_yptr_top_bak    = LCD_CacheBuffer_yptr_top;
      
      if(LCD_CacheBuffer_yptr_bottom > LCD_CacheBuffer_yptr_top) 
      {
        
        if ((LCD_CacheBuffer_yptr_bottom - LCD_CacheBuffer_yptr_top) <=  YWINDOW_SIZE)
        {
          LCD_Lock = DISABLE;
          return ERROR;
        }
      }
      LCD_ScrollActive = ENABLE;
      
      if((LCD_CacheBuffer_yptr_bottom  > LCD_CacheBuffer_yptr_top)&&
         (LCD_Scrolled == DISABLE ))
      {
        LCD_CacheBuffer_yptr_bottom--;
        LCD_Scrolled = ENABLE;
      }
      
    }
    
    if(LCD_ScrollActive == ENABLE)
    {
      LCD_Lock = ENABLE;
      LCD_ScrollBackStep--;
      
      if(++LCD_CacheBuffer_yptr_bottom == LCD_CACHE_DEPTH)
      {
        LCD_CacheBuffer_yptr_bottom = 0;
      }
      
      LCD_LOG_UpdateDisplay();
      LCD_Lock = DISABLE;
      
    } 
    return SUCCESS;
  }
  else // LCD_ScrollBackStep == 0 
  {
    LCD_Lock = DISABLE;
    return ERROR;
  }  
}
#endif /* LCD_SCROLL_ENABLED */

/**
  * @}
  */
  
/**
  * @}
  */ 

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
