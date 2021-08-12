int buzzer_time = 0;

void buzzer_callback(void)
{
     
    if(buzzer_time > 0)
    {
        buzzer_time --;
    }  
    else digitalWrite(BUZZER,0);
}

void buzzer_on(int time)
{
    digitalWrite(BUZZER,1);
    buzzer_time = time;
}