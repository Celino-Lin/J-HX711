#include <J_Hx711.h>

#define LedPin  13
#define CollectNum 5    //定义采集数量为5组

HX711 LC1;

long WeightSet[CollectNum] = {0,50,200,500,1000};//预设砝码的阶层，这是使用5组砝码，0g;50g;100g;150g;1000g;
double Collectbuffer[CollectNum]; 

void setup() {
  // put your setup code here, to run once:
  pinMode(LedPin, OUTPUT);
  Serial.begin(9600,SERIAL_8N1);
  LC1.initialization(2,3,91.99f,43504);

  Serial.println("Setup finished.");
}

void loop() {
  // put your main code here, to run repeatedly:
    if (Serial.available()>0) 
  {
    String CMDString = Serial.readString();
    Serial.println(CMDString); 

    if (CMDString =="test")
    {
      Monitoring();
    }
    else if (CMDString =="start")
    {
      if (LC1.validation()) 
      {
        Collectbuffer[0] = LC1.read_filter();
        Serial.print(WeightSet[0]);
        Serial.print("g collection value: ");
        Serial.println(Collectbuffer[0]);
        Serial.println("OK");
      }
      else
      {
        Collectbuffer[0] = 0;
        Serial.println("Error:LoadCell check execution error;Check if the connection is correct and the voltage is out of tolerance.");
      }
    }
    else if (CollectNum>1 && CMDString == "collect1")
    {
      if (Collectbuffer[0]!=0 )
      {
        Serial.print(WeightSet[1]);
        Serial.print("g collection value: ");
        Collectbuffer[1] = LC1.read_filter();
        Serial.println(Collectbuffer[1]);
        Serial.println("OK");
      }
      else Serial.println("Error:LoadCell collection failed,Please empty the overload on Loadcell and execute the 'start' command."); 
    }
    else if (CollectNum>2 && CMDString == "collect2")
    {
      if (Collectbuffer[0]!=0 && Collectbuffer[1]!=0)
      {
        //已放入砝码100g。
        Serial.print(WeightSet[2]);
        Serial.print("g collection value: ");
        Collectbuffer[2] = LC1.read_filter();
        Serial.println(Collectbuffer[2]);
        Serial.println("OK");
      }
      else Serial.println("Error:LoadCell collection failed,Please execute the 'collect1' command.");
            
    }
    else if (CollectNum>3 && CMDString == "collect3")
    {
      if (Collectbuffer[0]!=0 && Collectbuffer[2]!=0)
      {
        //已放入砝码150g。
        Serial.print(WeightSet[3]);
        Serial.print("g collection value: ");
        Collectbuffer[3] = LC1.read_filter();
        Serial.println(Collectbuffer[3]);
        Serial.println("OK");
      }
      else Serial.println("Error:LoadCell collection failed,Please execute the 'collect2' command.");
            
    }
    else if (CollectNum>4 && CMDString == "collect4")
    {
      if (Collectbuffer[0]!=0 && Collectbuffer[3]!=0)
      {
        //已放入砝码1000g。
        Serial.print(WeightSet[4]);
        Serial.print("g collection value: ");
        Collectbuffer[4] = LC1.read_filter();
        Serial.println(Collectbuffer[4]);
        Serial.println("OK");
      }
      else Serial.println("Error:LoadCell collection failed,Please execute the 'collect3' command.");

    }
    else if (CollectNum>1 && CMDString == "end")
    {
      if ( Collectbuffer[0]!=0 && Collectbuffer[CollectNum-1]!=0 )
      {
        //double PerGram = Collectbuffer[1]-Collectbuffer[0] / WeightSet[1]-WeightSet[0]; //预估每克多少读数。
        if (LC1.read_filter() < Collectbuffer[0]+Collectbuffer[0]/10) //确认盖子重量小于5g
        {
          LC1.CalculatedScale(WeightSet,Collectbuffer,CollectNum);//系数计算完成
          for(int i=0;i<CollectNum;i++)
          {
            Collectbuffer[i]=0;
          }
          do
          {
            LC1.CalculatedTare();//这个函数是用来归零的。
          }while(LC1.get_units()>0.02 || LC1.get_units()<-0.02);
          
          Serial.print("Scale: ");
          Serial.print(LC1.get_scale(),10);
          Serial.print("  OFFSET: ");
          Serial.println(LC1.get_offset());
          Serial.println("OK");
        }
        else Serial.println("Error:Please clear the weight on loadcell first. It cannot be pressed when the lid is closed");
      }
      else Serial.println("Error:Data collection is incomplete, execute other collection commands");
    }
    else
    {
      Serial.println("Error:Unknown command!!");
    }
  }
  delay(100);
  digitalWrite(LedPin, !digitalRead(LedPin));
}

void Monitoring(void)
{
  float FirstMeasure,SecondMeasure=0;
  int g=1;//这两个要放在循环外，不然每次都会归零
  char acount=0;
  float Amax=0,Amin =0xfff1;
  while(1)
  {
    FirstMeasure=LC1.get_units(5);
    if((FirstMeasure-SecondMeasure>SecondMeasure/10||SecondMeasure-FirstMeasure>SecondMeasure/10)&&(SecondMeasure>1||FirstMeasure>1))//变化大于原质量的10%时快速反应，保持稳定
    {
      SecondMeasure=LC1.get_units(5);
      g=1;
    }
    SecondMeasure=(g*SecondMeasure+FirstMeasure)/(g+1);
    if (g<10)
    {
      g++;
    }
    if (g=10)
    {
      Serial.println(SecondMeasure);
      if(Amax<SecondMeasure)  Amax=SecondMeasure;
      if(Amin>SecondMeasure)  Amin=SecondMeasure;
      if (acount==50)
      {//统计每50次测量数据中的最大最小值
        acount=0;
        Serial.print(" Max: ");
        Serial.print(Amax);
        Serial.print(" Min: ");
        Serial.print(Amin);
        Serial.print(" Offset: ");
        Serial.print(Amax-Amin);
        Serial.print(" Mid: ");
        Serial.println((Amax+Amin)/2);
        Amax=0;
        Amin =0xfff1;
        delay(100);
      }
      acount++;
      digitalWrite(LedPin, !digitalRead(LedPin));
    }
    if (Serial.available()>0)
    {
      String CMDString = Serial.readString();
      Serial.println(CMDString);
      if (CMDString =="exit")
      {
        return 0;
      }
      else
      {
        Serial.println("Error:Unknown command!!");
      }
    }
  }
}
