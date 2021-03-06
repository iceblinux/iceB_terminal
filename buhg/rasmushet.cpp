/* $Id: rasmushet.c,v 5.117 2013/11/24 08:23:03 sasa Exp $ */
/*19.06.2019    15.04.1997      Белых А.И.      rasmushet.c
Распечатка накладных, счета, Акта приемки
*/
#include        <errno.h>
#include        <math.h>
#include	"buhg.h"

void rasnak_vztr(short dd,short md,short gd,int skl,const char *nomdok,double *sumt,int simv_plus,FILE *f1);
double rasnak_end(int tipz,short dd,short md,short gd,const char *nomdok,int skl,int dlinna,double itogo,float pnds,int lnds,int mnt,int simv_plus,FILE *ff);

extern double	okrcn;  /*Округление цены*/
extern double   okrg1; /*Округление*/
extern short    srtnk; /*0-не включена 1-включена сортировка записей в накладной*/
extern short	vtara; /*Код группы возвратная тара*/
extern short    obzap; /*0-не объединять записи 1-обединять*/
extern class SPISOK sp_fio_klad; /*выбранные фамилии кладовщиков по складам*/
extern class masiv_din_int sp_kod_sklad; /*список кодов складов для фамилий кладовщиков*/

int rasmushet(short dg,short mg,short gg, //дата документа
int skl,  //Склад
const char *nomdok, //Номер документа
const char *imaf,   // имя файла
short lnds,  //Льготы по НДС*
int *simv_plus,
FILE *f1,double ves) //Вес
{
class iceb_tu_str nmo(""),nmo1("");  /*Наименование организации*/
class iceb_tu_str gor(""),gor1(""); /*Адрес*/
class iceb_tu_str pkod(""),pkod1(""); /*Почтовый код*/
class iceb_tu_str nmb(""),nmb1("");  /*Наименование банка*/
class iceb_tu_str mfo(""),mfo1("");  /*МФО*/
class iceb_tu_str nsh(""),nsh1("");  /*Номер счета*/
class iceb_tu_str inn(""),inn1("");  /*Индивидуальный налоговый номер*/
class iceb_tu_str grb(""),grb1("");  /*Город банка*/
class iceb_tu_str npnds(""),npnds1(""); /*Номер плательщика НДС*/
class iceb_tu_str tel(""),tel1("");    /*Номер телефона*/
class iceb_tu_str gor_kontr("");
class iceb_tu_str gor_kontr1("");
class iceb_tu_str regnom(""); /*Регистрационный номер частного предпринимателя*/
char            bros1[1024];
int             i;
class iceb_tu_str nomn("");  /*Номенклатурный номер*/
double          itogo;
double          itogo1;
double          bb=0.,bb1=0.;
short           d,m,g;
short           dd,md,gd;
//short           nnds=0; //0-накладная 1-накладная с ценой НДС 2-накладная с номером заказа
short		mnt; /*Метка наличия тары*/
double		sumt; /*Сумма по таре*/
class iceb_tu_str kdtr(""); /*Коды тары*/
short		mtpr; /*Метка первого и второго прохода*/
short		mppu; /*Метка первого прохода услуг*/
double		mest;
class iceb_tu_str naimpr("");
int		skl1;
short           mvnpp; /*0-внешняя 1 -внутреняя 2-изменение стоимости*/
int             kodm,kodmz,nk;
double		kol,sum;
double		kolt=0.;
double		kratn,kratz,cenan,cenaz;
class iceb_tu_str ein(""),eiz("");
class iceb_tu_str naim(""),naimz("");
//short		klst,klli;
SQL_str         row,row1,rowtmp;
SQLCURSOR cur,cur1,curtmp;
char		strsql[2048];
long		kolstr;
class iceb_tu_str kpos("");
class iceb_tu_str kop(""); /*Код операции*/
int		tipz=0;
class iceb_tu_str naiskl("");
class iceb_tu_str nomnn(""); /*Номер налоговой накладной*/
class iceb_tu_str nn1(""); /*Номер встречного документа*/
char		bros[1024];
char		str[1024];
short		vtar; /*Метка возвратной тары*/
short		kgrm; /*код группы материалла*/
class iceb_tu_str shu(""); /*счет учета материалла*/
double		kolih,cena;
class iceb_tu_str fmoll(""); //Фамилия материально-ответственного
class iceb_tu_str nomz("");
class iceb_tu_str nomzz("");
class iceb_tu_str mesqc("");
class iceb_tu_str kodkontr00("00");
short m_tara=0; //Не возвратная тара- обычная
double itogo_mest=0.;
mest=mnt=0;

/*Читаем шапку документа*/
sprintf(strsql,"select * from Dokummat \
where datd='%04d-%02d-%02d' and sklad=%d and nomd='%s'",
gg,mg,dg,skl,nomdok);

if(readkey(strsql,&row,&cur) != 1)
 {
  VVOD SOOB(1);

  sprintf(strsql,"%s-%s",__FUNCTION__,gettext("Не найден документ !")),
  SOOB.VVOD_SPISOK_add_MD(strsql);

  sprintf(strsql,"%s=%d.%d.%d %s=%s %s=%d",
  gettext("Дата"),
  dg,mg,gg,
  gettext("Номер документа"),
  nomdok,
  gettext("Код склада"),
  skl);
  SOOB.VVOD_SPISOK_add_MD(strsql);

  soobshw(&SOOB,stdscr,-1,-1,0,1);
  return(1);
 }
float pnds=atof(row[13]);
dd=dg;
md=mg;
gd=gg;
kpos.new_plus(row[3]);
kop.new_plus(row[6]);
tipz=atoi(row[0]);
nomnn.new_plus(row[5]);
nn1.new_plus(row[11]);

kodkontr00.new_plus(row[15]);

/*Читаем наименование операции*/
if(tipz == 1)
 strcpy(bros,"Prihod");
if(tipz == 2)
 strcpy(bros,"Rashod");

mvnpp=0;

sprintf(strsql,"select naik,vido from %s where kod='%s'",bros,kop.ravno());
if(readkey(strsql,&row,&cur) != 1)
 {

  sprintf(strsql,gettext("Не найден код операции %s !"),kop.ravno());
  iceb_t_soob(strsql);

 }
else
 {

  mvnpp=atoi(row[1]);
  naimpr.new_plus(row[0]);
  
 }

class iceb_tu_str nadpis("");
iceb_t_poldan("Надпись на расходной накладной",&nadpis,"matnast.alx");
  


iceb_t_poldan("Код тары",&kdtr,"matnast.alx");


if(tipz == 2)
 {
  if(SRAV("00",kodkontr00.ravno(),0) == 0)
    kodkontr00.new_plus(iceb_t_getk00(0,kop.ravno()));
 }

/*Читаем реквизиты организации свои */


sprintf(bros,"select * from Kontragent where kodkon='%s'",kodkontr00.ravno());
if(readkey(bros,&row,&cur) != 1)
 {
  sprintf(strsql,"%s %s !",gettext("Не найден код контрагента"),kodkontr00.ravno());
  iceb_t_soob(strsql);
 }
else
 {
  if(row[16][0] == '\0')
   {
    if(polen(row[1],&nmo,1,'(') != 0)
     nmo.new_plus(row[1]);
   }
  else
    nmo.new_plus(row[16]);
    
  gor.new_plus(row[3]);
  pkod.new_plus(row[5]);
  nmb.new_plus(row[2]);
  mfo.new_plus(row[6]);
  nsh.new_plus(row[7]);
  inn.new_plus(row[8]);
  grb.new_plus(row[4]);
  npnds.new_plus(row[9]);
  tel.new_plus(row[10]);
  gor_kontr.new_plus(row[17]);
 } 


/*Читаем реквизиты организации чужие*/
/*При внутреннем перемещении не читать*/
if(ATOFM(kpos.ravno()) != 0. || kpos.ravno()[0] != '0')
 {

  sprintf(bros,"select * from Kontragent where kodkon='%s'",kpos.ravno());
  if(readkey(bros,&row,&cur) != 1)
   {
    sprintf(strsql,"%s %s !",gettext("Не найден код контрагента"),kpos.ravno());
    iceb_t_soob(strsql);
   }
  else
   {
    if(row[16][0] == '\0')
     nmo1.new_plus(row[1]);
    else
     nmo1.new_plus(row[16]);
    
    gor1.new_plus(row[3]);
    pkod1.new_plus(row[5]);
    nmb1.new_plus(row[2]);
    mfo1.new_plus(row[6]);
    nsh1.new_plus(row[7]);
    inn1.new_plus(row[8]);
    grb1.new_plus(row[4]);
    npnds1.new_plus(row[9]);
    tel1.new_plus(row[10]);
    regnom.new_plus(row[15]);
    gor_kontr1.new_plus(row[17]);    
   }
 }

class iceb_t_tmptab tabtmp;
const char *imatmptab={__FUNCTION__};
int kol_znak_nomn=0;
if(sortdokmuw(dg,mg,gg,skl,nomdok,imatmptab,&tabtmp,&kol_znak_nomn) != 0)
 return(1);


sprintf(strsql,"select * from %s",imatmptab);

if(srtnk == 1)
  sprintf(strsql,"select * from %s order by kgrm asc,naim asc",imatmptab);

int kolstrtmp=0;
if((kolstrtmp=curtmp.make_cursor(&bd,strsql)) < 0)
 {
  msql_error(&bd,gettext("Ошибка создания курсора!"),strsql);
  return(1);
 }

int metka_open_fil=0;
if(f1 == NULL)
 {
  
  if((f1 = fopen(imaf,"w")) == NULL)
   {
    error_op_nfil(imaf,errno,"");
    return(1);
   }
  metka_open_fil=1;
 }

//fprintf(f1,"\x1B\x33%c",30-kor); /*Уменьшаем межстрочный интервал*/
//fprintf(f1,"\x12"); /*Отмена ужатого режима*/
//fprintf(f1,"\x1B\x4D"); /*12-знаков*/

fprintf(f1,"\x1B\x45"); /*Включение режима выделенной печати*/


if(nadpis.ravno()[0] != '\0' && tipz == 2 && mvnpp == 0)
   fprintf(f1,"%s\n",nadpis.ravno());
  
if(tipz == 2)
 fprintf(f1,gettext("\
        Поставщик                                                 Плательщик\n"));
if(tipz == 1)
 fprintf(f1,gettext("\
        Плательщик                                                Поставщик\n"));

fprintf(f1,"%-*.*s N%s %-*.*s\n",
iceb_tu_kolbait(60,nmo.ravno()),iceb_tu_kolbait(60,nmo.ravno()),nmo.ravno(),
kpos.ravno(),
iceb_tu_kolbait(60,nmo1.ravno()),iceb_tu_kolbait(60,nmo1.ravno()),nmo1.ravno());

for(int nom=60; nom < iceb_tu_strlen(nmo.ravno()) || nom < iceb_tu_strlen(nmo1.ravno()); nom+=60)  
 {
  if(nom < iceb_tu_strlen(nmo.ravno()))
   {  
    fprintf(f1,"%-*.*s",
    iceb_tu_kolbait(60,iceb_tu_adrsimv(nom,nmo.ravno())),
    iceb_tu_kolbait(60,iceb_tu_adrsimv(nom,nmo.ravno())),
    iceb_tu_adrsimv(nom,nmo.ravno()));

   }
  else
    fprintf(f1,"%60s "," ");
    
  if(nom < iceb_tu_strlen(nmo1.ravno()))
   {
    fprintf(f1,"%-*.*s",
    iceb_tu_kolbait(60,iceb_tu_adrsimv(nom,nmo1.ravno())),
    iceb_tu_kolbait(60,iceb_tu_adrsimv(nom,nmo1.ravno())),
    iceb_tu_adrsimv(nom,nmo1.ravno()));
   }
  fprintf(f1,"\n");
 }   



memset(bros,'\0',sizeof(bros));
memset(bros1,'\0',sizeof(bros1));
sprintf(bros,"%s %s",gettext("Адрес"),gor.ravno());
sprintf(bros1,"%s %s",gettext("Адрес"),gor1.ravno());

fprintf(f1,"%-*.*s %-*.*s\n",
iceb_tu_kolbait(60,bros),iceb_tu_kolbait(60,bros),bros,
iceb_tu_kolbait(60,bros1),iceb_tu_kolbait(60,bros1),bros1);

if(iceb_tu_strlen(bros) > 60 || iceb_tu_strlen(bros1) > 60)
 fprintf(f1,"%-*.*s %-*.*s\n",
 iceb_tu_kolbait(60,iceb_tu_adrsimv(60,bros)),
 iceb_tu_kolbait(60,iceb_tu_adrsimv(60,bros)),
 iceb_tu_adrsimv(60,bros),
 iceb_tu_kolbait(60,iceb_tu_adrsimv(60,bros1)),
 iceb_tu_kolbait(60,iceb_tu_adrsimv(60,bros1)),
 iceb_tu_adrsimv(60,bros1));
  

sprintf(bros,"%s %s",gettext("Код ЕГРПОУ"),pkod.ravno());
sprintf(bros1,"%s %s",gettext("Код ЕГРПОУ"),pkod1.ravno());
fprintf(f1,"%-*s %s\n",iceb_tu_kolbait(60,bros),bros,bros1);

sprintf(bros,"%s %s %s %s",gettext("Р/С"),nsh.ravno(),
gettext("МФО"),mfo.ravno());

sprintf(bros1,"%s %s %s %s",gettext("Р/С"),nsh1.ravno(),
gettext("МФО"),mfo1.ravno());

fprintf(f1,"%-*s %s\n",iceb_tu_kolbait(60,bros),bros,bros1);


sprintf(bros,"%s %s",gettext("в"),nmb.ravno());
sprintf(bros1,"%s %s",gettext("в"),nmb1.ravno());

if(grb.getdlinna() > 1)
 {
  sprintf(strsql," %s",grb.ravno());
  strcat(bros,strsql);
 }

if(grb1.getdlinna() > 1)
 {
  sprintf(strsql," %s",grb1.ravno());
  strcat(bros1,strsql);
 }

//sprintf(bros,"%s %s %s %s",gettext("в"),nmb.ravno(),gettext("гор."),grb.ravno());
//sprintf(bros1,"%s %s %s %s",gettext("в"),nmb1.ravno(),gettext("гор."),grb1.ravno());

fprintf(f1,"%-*.*s %s\n",iceb_tu_kolbait(60,bros),iceb_tu_kolbait(60,bros),bros,bros1);
if(iceb_tu_strlen(bros) > 60)
   fprintf(f1,"%s\n",iceb_tu_adrsimv(60,bros));  

sprintf(bros,"%s %s",
gettext("Налоговый номер"),inn.ravno());

sprintf(bros1,"%s %s",
gettext("Налоговый номер"),inn1.ravno());

fprintf(f1,"%-*s %s\n",iceb_tu_kolbait(60,bros),bros,bros1);

sprintf(bros,"%s %s",
gettext("Номер сви-ства плательщика НДС"),npnds.ravno());
sprintf(bros1,"%s %s",
gettext("Номер сви-ства плательщика НДС"),npnds1.ravno());
fprintf(f1,"%-*s %s\n",iceb_tu_kolbait(60,bros),bros,bros1);

sprintf(bros,"%s %s",gettext("Телефон"),tel.ravno());

sprintf(bros1,"%s %s",gettext("Телефон"),tel1.ravno());
fprintf(f1,"%-*s %s\n",iceb_tu_kolbait(60,bros),bros,bros1);


//fprintf(f1,"\x1B\x50"); /*10-знаков*/



class SPISOK fio_klad;
class iceb_tu_str fio_klad_vibr("");


/*Читаем наименование склада*/
sprintf(strsql,"select naik,fmol,np from Sklad where kod=%d",skl);
if(readkey(strsql,&row,&cur) != 1)
 {
  sprintf(strsql,gettext("Не найден склад %d в списке складов !"),skl);
  iceb_t_soob(strsql);
 }
else
 {


  naiskl.new_plus(row[0]);
  fmoll.new_plus(row[1]);


  if(row[2][0] != '\0')
    gor_kontr.new_plus(row[2]);

  int nom_skl=0;
  if((nom_skl=sp_kod_sklad.find(skl)) < 0)
   {
    iceb_tu_strspisok(row[1],'/',&fio_klad); /*составляем список фамилий кладовщиков*/

    if(fio_klad.kolih() == 1)
     {
      fio_klad_vibr.new_plus(fio_klad.ravno(0));
     }
    if(fio_klad.kolih() > 1)
     {
      int vozv=0;
      int kk=0;

      if((vozv=dirmasiv(&fio_klad,-1,-1,0,gettext("Выберите фамилию кладовщика"),0,&kk)) >= 0)
        fio_klad_vibr.new_plus(fio_klad.ravno(vozv));
        
     }

    sp_kod_sklad.plus(skl);
    sp_fio_klad.plus(fio_klad_vibr.ravno());
   
   }
  else
    fio_klad_vibr.new_plus(sp_fio_klad.ravno(nom_skl));
 }


if(mvnpp == 1) /*Внутреннее перемещение*/
 {
  if(ATOFM(kpos.ravno()) == 0. && polen(kpos.ravno(),bros,sizeof(bros),2,'-') == 0 && kpos.ravno()[0] == '0')
   {
    polen(kpos.ravno(),bros,sizeof(bros),2,'-');
    skl1=(int)ATOFM(bros);
    /*Читаем наименование склада*/
    sprintf(strsql,"select naik,fmol from Sklad where kod=%d",skl1);
    naiskl.new_plus("");
    class iceb_tu_str fmol("");
    if(readkey(strsql,&row,&cur) != 1)
     {
      move(20,0);
      beep();
      printw(gettext("Не найден склад %d в списке складов !"),skl1);
      printw("\n");
      OSTANOV();
     }
    else
     {
      naiskl.new_plus(row[0]);
      fmol.new_plus(row[1]);
     }
    if(tipz == 1)
     {
      
      sprintf(strsql,"%s: %s %d <= %s %d %s",
      naimpr.ravno(),gettext("Склад"),skl,
      gettext("Склад"),skl1,naiskl.ravno());
      
      fprintf(f1,"\n\%.*s\n",iceb_tu_kolbait(70,strsql),strsql);

      sprintf(strsql,"%22s %s <= %s"," ",fmoll.ravno(),fmol.ravno());
      fprintf(f1,"%.*s\n",iceb_tu_kolbait(70,strsql),strsql);
            
     }

    if(tipz == 2)
     {
      sprintf(strsql,"%s: %s %d => %s %d %s",
      naimpr.ravno(),gettext("Склад"),skl,
      gettext("Склад"),skl1,naiskl.ravno());

      fprintf(f1,"\n\%.*s\n",iceb_tu_kolbait(70,strsql),strsql);

      sprintf(strsql,"%22s %s >= %s"," ",fmoll.ravno(),fmol.ravno());
      fprintf(f1,"%.*s\n",iceb_tu_kolbait(70,strsql),strsql);

     }
   }
  else
    fprintf(f1,"%s\n",naimpr.ravno());
 }
else
 { 
  bros[0]='\0';

  if(iceb_t_poldan("Наименование операции в накладной",bros,"matnast.alx") == 0)
   if(SRAV(bros,"Вкл",1) == 0)
    fprintf(f1,"%s\n",naimpr.ravno());
   
 }


sprintf(bros,"N%s",nomdok);  
if(tipz == 2 && iceb_t_poldan("Перенос символа N",bros,"matnast.alx") == 0)
 {
  if(SRAV(bros,"Вкл",1) == 0)
   {
    memset(bros,'\0',sizeof(bros));
    polen(nomdok,bros,sizeof(bros),1,'-');
    polen(nomdok,bros1,sizeof(bros1),2,'-');
    strcat(bros,"-N");  
    strcat(bros,bros1);  
    
   }       
  else
    sprintf(bros,"N%s",nomdok);  

 } 
mesc(md,1,&mesqc);

if(tipz == 1)
 fprintf(f1,"%s\n",gor_kontr1.ravno());

if(tipz == 2) 
 fprintf(f1,"%s\n",gor_kontr.ravno());
/*************** 
fprintf(f1,"\
                                         %s %s\n",gettext("С ч ё т"),bros);
******************/
fprintf(f1,"\
                                   %s %s %s %d %s %d%s\n",
gettext("С ч ё т"),bros,gettext("от"),
dd,mesqc.ravno(),gd,gettext("г."));


/*Читаем доверенность*/
sprintf(strsql,"select sodz from Dokummat2 \
where god=%d and nomd='%s' and sklad=%d and nomerz=%d",
gg,nomdok,skl,1);

if(readkey(strsql,&row,&cur) == 1)
 {
  fprintf(f1,"%s N",gettext("Доверенность"));
  polen(row[0],bros,sizeof(bros),1,'#');
  fprintf(f1," %s",bros);
  polen(row[0],bros,sizeof(bros),2,'#');
  fprintf(f1," %s %s\n",gettext("от"),bros);
 }



/*Читаем через кого*/
class iceb_tu_str sherez_kogo("");
sprintf(strsql,"select sodz from Dokummat2 \
where god=%d and nomd='%s' and sklad=%d and nomerz=%d",
gg,nomdok,skl,2);

if(readkey(strsql,&row,&cur) == 1)
 {
  fprintf(f1,"%s: %s\n",gettext("Через кого"),row[0]);
  sherez_kogo.new_plus(row[0]);
 }

if(tipz == 2)
 {
  sprintf(strsql,"select sodz from Dokummat2 \
where god=%d and nomd='%s' and sklad=%d and nomerz=%d",
  gg,nomdok,skl,3);

  if(readkey(strsql,&row,&cur) == 1)
    fprintf(f1,"%s: %s\n",gettext("Основание"),row[0]);

  if(regnom.ravno()[0] != '\0')
    fprintf(f1,"%s: %s\n",gettext("Регистрационный номер"),regnom.ravno());
  
  sprintf(strsql,"select sodz from Dokummat2 \
where god=%d and nomd='%s' and sklad=%d and nomerz=%d",
  gg,nomdok,skl,9);

  if(readkey(strsql,&row,&cur) == 1)
   {
    rsdat(&d,&m,&g,row[0],0);
//    if(d == 0 || SRAV1(g,m,d,gd,md,dd) <= 0)
    if(d == 0 || sravmydat(d,m,g,dd,md,gd) >= 0)
     {
      fprintf(f1,"\x1B\x45%s %s\x1B\x45\x1b\x46\n",
      gettext("Счёт действителен до"),row[0]);

     }
   }

  sprintf(strsql,"select sodz from Dokummat2 \
where god=%d and nomd='%s' and sklad=%d and nomerz=%d",
  gg,nomdok,skl,10);

  if(readkey(strsql,&row,&cur) == 1)

  if(nomnn.ravno()[0] != '\0')
    fprintf(f1,"%s: %s\n",
    gettext("Номер налоговой накладной"),nomnn.ravno());
    
 }

if(nn1.ravno()[0] != '\0' && tipz == 1)
 fprintf(f1,"%s: %s\n",
 gettext("Номер встречной накладной"),nn1.ravno());
if(nn1.ravno()[0] != '\0' && tipz == 2 && mvnpp > 0 )
 fprintf(f1,"%s: %s\n",
 gettext("Номер встречной накладной"),nn1.ravno());

fprintf(f1,"\x1B\x46"); /*Выключение режима выделенной печати*/
fprintf(f1,"\x1B\x47"); /*Включение режима двойного удара*/

//fprintf(f1,"\x0F"); //Включение ужатого режима печати


class iceb_rnl_c rh;

if(kol_znak_nomn > 18)
 {
  *simv_plus=rh.simv_plus=kol_znak_nomn-18;
 }

rasmushet_sap(&rh,NULL,f1);

itogo=itogo1=0.;
mtpr=0;

naz:;
mnt=0;
i=0;
kol=kolt=sum=0.;
nomn.new_plus("");
kratn=cenan=kratz=cenaz=kodmz=0;

while(curtmp.read_cursor(&rowtmp) != 0)
 {

  kgrm=atoi(rowtmp[0]);
  naim.new_plus(rowtmp[1]);
  kodm=atoi(rowtmp[2]);
  nk=atoi(rowtmp[3]);
  cenan=atof(rowtmp[4]);
  cenan=okrug(cenan,okrcn);
  kratn=atoi(rowtmp[5]);
  ein.new_plus(rowtmp[6]);
  vtar=atoi(rowtmp[7]);
  shu.new_plus(rowtmp[8]);
  kolih=atof(rowtmp[9]);
  nomz.new_plus(rowtmp[10]);
   
  if(vtar == 1)
   {
    mnt++;
    continue;
   } 

  mest=0;
//  kolt=0.;

  if(vtara != 0 && kgrm == vtara)
   {
    mnt++;
    continue;
   } 

  memset(bros,'\0',sizeof(bros));
  sprintf(bros,"%d",kodm);
  if(mtpr == 0)
   if(proverka(kdtr.ravno(),bros,0,1) == 0)
    {  
     m_tara++;
     continue;
    }
  if(mtpr == 1)
   if(proverka(kdtr.ravno(),bros,0,1) != 0)
    continue;
    
  if(obzap == 1)
   {
    if((kodmz != 0 && kodmz != kodm) || (kratz != 0 && kratz != kratn) ||
    (cenaz != 0. && cenaz != cenan) || (eiz.ravno()[0] != '\0' && SRAV(eiz.ravno(),ein.ravno(),0) != 0)\
     || (naimz.ravno()[0] != '\0' && SRAV(naimz.ravno(),naim.ravno(),0) != 0))
     {
      i++;
      fprintf(f1,"%3d %-*s %-*.*s %-*.*s %10.10g %11s",
      i,
      iceb_tu_kolbait(18+*simv_plus,nomn.ravno()),nomn.ravno(),
      iceb_tu_kolbait(46,naimz.ravno()),iceb_tu_kolbait(46,naimz.ravno()),naimz.ravno(),
      iceb_tu_kolbait(4,eiz.ravno()),iceb_tu_kolbait(4,eiz.ravno()),eiz.ravno(),
      kol,prcn(cenaz));
      
      fprintf(f1," %10s",prcn(sum));
      rasdokkr(kol,kolt,kratz,f1);

      if(iceb_tu_strlen(naimz.ravno()) > 46)
       {
        fprintf(f1,"%3s %*s %s\n"," ",18+*simv_plus," ",iceb_tu_adrsimv(46,naimz.ravno()));
       }
     }
    if(kodmz != kodm || kratz != kratn || cenaz != cenan || 
     SRAV(eiz.ravno(),ein.ravno(),0) != 0 || SRAV(naimz.ravno(),naim.ravno(),0) != 0)
     {
      kol=kolt=sum=0.;
      kodmz=kodm;
      cenaz=cenan;
      kratz=kratn;
      eiz.new_plus(ein.ravno());
      naimz.new_plus(naim.ravno());

     }
   }
  sprintf(strsql,"%d.%s.%d.%d",skl,shu.ravno(),kodm,nk);

  if(obzap == 1 && kol != 0.)
     sprintf(strsql,"%d.%s.%d.***",skl,shu.ravno(),kodm);
  nomn.new_plus(strsql);
  nomzz.new_plus(nomz.ravno());
  bb=cenan*kolih;
  bb=okrug(bb,okrg1);
  bb1=cenan+(cenan*pnds/100.);
  bb1=okrug(bb1,okrg1);

  mest=0;
  if(mtpr == 0 && kratn != 0.)
   {
    mest=kolih/kratn;
    mest=okrug(mest,0.1);
    itogo_mest+=mest;
   }

  if(obzap == 0)
   {
    naimz.new_plus(naim.ravno());

      i++;
      fprintf(f1,"%3d %-*s %-*.*s %-*.*s %10.10g %11s",
      i,
      iceb_tu_kolbait(18+*simv_plus,nomn.ravno()),nomn.ravno(),
      iceb_tu_kolbait(46,naim.ravno()),iceb_tu_kolbait(46,naim.ravno()),naim.ravno(),
      iceb_tu_kolbait(4,ein.ravno()),iceb_tu_kolbait(4,ein.ravno()),ein.ravno(),
      kolih,prcn(cenan));
      
      fprintf(f1," %10s",prcn(bb));
      rasdokkr(kol,mest,kratn,f1);
      if(iceb_tu_strlen(naim.ravno()) > 46)
       {
        fprintf(f1,"%3s %15s %s\n"," "," ",iceb_tu_adrsimv(46,naim.ravno()));
       }
   }

  kol+=kolih;
  kolt+=mest;
  sum+=bb;
  itogo+=bb;


 }

if(obzap == 1)
 {
  i++;

  fprintf(f1,"%3d %-*s %-*.*s %-*.*s %10.10g %11s",
  i,
  iceb_tu_kolbait(18+*simv_plus,nomn.ravno()),nomn.ravno(),
  iceb_tu_kolbait(46,naimz.ravno()),iceb_tu_kolbait(46,naimz.ravno()),naimz.ravno(),
  iceb_tu_kolbait(4,eiz.ravno()),iceb_tu_kolbait(4,eiz.ravno()),eiz.ravno(),
  kol,prcn(cenaz));
  fprintf(f1," %10s",prcn(sum));
  rasdokkr(kol,kolt,kratz,f1);
  if(iceb_tu_strlen(naimz.ravno()) > 46)
   {
    fprintf(f1,"%3s %15s %s\n"," "," ",iceb_tu_adrsimv(46,naimz.ravno()));
   }
 }

mtpr++;

if(mtpr == 1 && m_tara != 0) /*Распечатываем отдельно тару*/
 {
  fprintf(f1,"\
- - - - - - - - - - - - - - - - - - - - %s - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n",
    gettext("Т а р а"));


  curtmp.poz_cursor(0);
  goto naz;
 }


/*Распечатываем услуги*/
sprintf(strsql,"select * from Dokummat3 where \
datd='%d-%02d-%02d' and sklad=%d and nomd='%s'",gd,md,dd,skl,nomdok);
/*printw("\nstrsql=%s\n",strsql);*/
if((kolstr=cur.make_cursor(&bd,strsql)) < 0)
 {
  msql_error(&bd,gettext("Ошибка создания курсора !"),strsql);
  return(1);
 }

mppu=0;/*метка первого прохода услуг*/
class iceb_tu_str naiusl("");

if(kolstr != 0)
while(cur.read_cursor(&row) != 0)
 {
  kolih=atof(row[4]);
  cena=atof(row[5]);
  cena=okrug(cena,okrcn);
  
  nomn.new_plus(gettext("**Услуги***"));
  if(kolih > 0)
    bb=cena*kolih;
  else
    bb=cena;
  
  bb=okrug(bb,okrg1);
  bb1=cena+(cena*pnds/100.);
  bb1=okrug(bb1,okrg1);

  naiusl.new_plus("");
  if(atoi(row[10]) != 0)
   {
    sprintf(strsql,"select naius from Uslugi where kodus=%s",row[10]);
    if(readkey(strsql,&row1,&cur1) == 1)
     naiusl.new_plus(row1[0]);
   }  
  if(naiusl.getdlinna() <= 1)
   naiusl.new_plus(row[7]);
  else
   {
    naiusl.plus(" ",row[7]);
   }

    fprintf(f1,"%3d %-*s %-*.*s %-*.*s %10.10g %11s",
    ++i,
    iceb_tu_kolbait(18+*simv_plus,nomn.ravno()),nomn.ravno(),
    iceb_tu_kolbait(46,naiusl.ravno()),iceb_tu_kolbait(46,naiusl.ravno()),naiusl.ravno(),
    iceb_tu_kolbait(4,row[3]),iceb_tu_kolbait(4,row[3]),row[3],
    kolih,prcn(cena));
    fprintf(f1," %10s\n",prcn(bb));
  
  mppu++;
  
  itogo+=bb;

 }

fprintf(f1,"ICEB_LST_END\n");

int dlinna=0; 

memset(strsql,'\0',sizeof(strsql));
if(itogo_mest > 0.)
   sprintf(strsql,"%.f",itogo_mest);

dlinna=96+*simv_plus;
fprintf(f1,"\
------------------------------------------------------------------------------------------------------------------------\n\
%*s: %10s %-4s\n",iceb_tu_kolbait(dlinna,gettext("Итого")),gettext("Итого"),prcn(itogo),strsql);


itogo1=itogo;

if(mvnpp == 0) /*Внешнее перемещение*/
 {
  
  itogo1=rasnak_end(tipz,dd,md,gd,nomdok,skl,dlinna,itogo,pnds,lnds,mnt,*simv_plus,f1);

 }
else
 { 
  sumt=0.;
  if(mnt != 0)
     rasnak_vztr(dd,md,gd,skl,nomdok,&sumt,*simv_plus,f1);
  itogo1+=sumt;
 }

memset(str,'\0',sizeof(str));
preobr(itogo1,str,0);


fprintf(f1,"%s:%s\n",gettext("Сумма прописью"),str);
if(lnds != 0 && mvnpp == 0 )
 fprintf(f1,"%s\n",gettext("Без НДС"));
 
//fprintf(f1,"\x1B\x50"); /*10-знаков*/
//fprintf(f1,"\x1B\x32"); /*Межстрочный интервал 1/6 дюйма*/

class iceb_t_fioruk_rk rukov;
class iceb_t_fioruk_rk glavbuh;
iceb_t_fioruk(1,&rukov);
iceb_t_fioruk(2,&glavbuh);


//fprintf(f1,"\x12"); /*Отмена ужатого режима печати*/


fprintf(f1,"\n%*s________________%s\n\n%*s________________%s\n",
iceb_tu_kolbait(20,gettext("Руководитель")),gettext("Руководитель"),
rukov.famil_inic.ravno(),
iceb_tu_kolbait(20,gettext("Главный бухгалтер")),gettext("Главный бухгалтер"),
glavbuh.famil_inic.ravno());

iceb_t_insfil("matshetend.alx",f1);
fprintf(f1,"\x1B\x48"); /*Выключение режима двойного удара*/



if(mvnpp == 0 && tipz == 2) /*Внешнее перемещение*/
 {
  if(nalndoh() == 0)
    fprintf(f1,"%s\n",gettext("Предприятие является плательщиком налога на доход на общих основаниях."));
//  fprintf(f1,"\n");
  if(ves != 0)
    fprintf(f1,"%s: %.2f %s\n",
    gettext("Вес"),ves,gettext("Кг."));

  fprintf(f1,"\x0E"); /*Включение режима удвоенной ширины*/
  fprintf(f1,gettext("Благодарим за сотрудничество !"));
  fprintf(f1,"\x14"); /*Выключение режима удвоенной ширины*/
 }


podpis(f1);


clearstr((short)(LINES-2),0);
printw("%s \"%s\"",
gettext("Распечатка счета выгружена в файл"),
imaf);

if(metka_open_fil == 1)
 {
  fclose(f1);
  class iceb_rnl_c rh;
  iceb_t_ustpeh(imaf,3,&rh.orient);
  rh.simv_plus=*simv_plus;
  iceb_t_rnl(imaf,&rh,&rasmushet_sap);
 } 
return(0);
}

/************************/
/*Выдача шапки накладной*/
/************************/
void rasmushet_sap(class iceb_rnl_c *rh,int *nom_str,FILE *ff)
{
if(nom_str != NULL)
  *nom_str+=4;


for(int nom=0; nom < rh->simv_plus; nom++)
 fprintf(ff,"-");
 
fprintf(ff,"\
------------------------------------------------------------------------------------------------------------------------\n");

//fprintf(ff,gettext(" N |  Номенклатурный  |       Наименование продукции                 |Ед. |Количество|  Ц е н а  | С у м м а|Кол.|Крат-|\n"));
//fprintf(ff,gettext("   |       номер      |       (товаров,работ,услуг)                  |изм.|          |           |          |м-ст|ность|\n"));


fprintf(ff,"%s",gettext(" N |  Номенклатурный  "));

for(int nom=0; nom < rh->simv_plus; nom++)
 fprintf(ff," ");

fprintf(ff,"%s",gettext("|       Наименование продукции                 |Ед. |Количество|  Ц е н а  | С у м м а|Кол.|Крат-|\n"));

fprintf(ff,"%s",gettext("   |       номер      "));

for(int nom=0; nom < rh->simv_plus; nom++)
 fprintf(ff," ");

fprintf(ff,"%s",gettext("|       (товаров,работ,услуг)                  |изм.|          |           |          |м-ст|ность|\n"));

for(int nom=0; nom < rh->simv_plus; nom++)
 fprintf(ff,"-");

fprintf(ff,"\
------------------------------------------------------------------------------------------------------------------------\n");
}
