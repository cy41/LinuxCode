#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<linux/sched.h>
#include<sys/wait.h>
#include<errno.h>
#include<bits/stdc++.h>
#include<sys/stat.h>
#include<signal.h>
#include<sys/sem.h>
/*
#include <sys/sem.h>
// 创建或获取一个信号量组：若成功返回信号量集ID，失败返回-1
int semget(key_t key, int num_sems, int sem_flags);
// 对信号量组进行操作，改变信号量的值：成功返回0，失败返回-1
int semop(int semid, struct sembuf semoparray[], size_t numops);
// 控制信号量的相关信息
int semctl(int semid, int sem_num, int cmd, ...);
*/


using namespace std;

#define debug(x) cout<<(#x)<<" : "<<(x)

#define MAXROW 100 //最大进程数;
#define MAXCAL 100 //单个进程可以申请的最大资源量;
#define MAXSIZE 200//管程通信中单次允许的最大字符流长度;


char buf[MAXSIZE]; //管程通信数组;

int Max[MAXROW][MAXCAL];        //最大需求;
int Allocation[MAXROW][MAXCAL]; //已分配资源;
int Need[MAXROW][MAXCAL];       //需求;
pid_t ID[MAXROW];               //记录子进程ｉｄ,若完成分配则置０;
int num;                        //存活的子进程的数量;
pid_t thisid;                   //子进程的pid;

int Available[MAXCAL];          //拥有资源数;
int n,m;                        //进程数，资源数;
int id;                         //标志当前进程为几号进程;
int Request[MAXCAL];            //请求资源量;

/*------------函数说明区--------------*/

/*
vector<int> getal();                 //将字符串拆解为数字数组;
void itoa(char s[],int val,int& pos);//将数字ｖａｌ从ｐｏｓ位置开始拼接到ｓ后;
bool issafe();                       //安全性检测;
int check(int Request[],int id);     //银行家算法;
void output_check(int i);            //输出进程请求后的信息;
void pri();                          //打印当前所有进程的资源矩阵;
int init_sem(int sem_id, int value); // 初始化信号量;
int sem_p(int sem_id);               // P操作:
int sem_v(int sem_id);               // V操作;
int del_sem(int sem_id);             // 删除信号量集;
void output_request(vector<int> v);  //输出每次请求的进程所请求的资源;
void check_process();                //检测是否有进程已经获得了所有资源可以结束掉了;
*/

/*------------函数说明区--------------*/

//将字符串拆解为数字数组;
vector<int> getval(){
    int n=strlen(buf);
    int x=0;
    vector<int> v;
    v.clear();
    for(int i=0;i<n;++i){
        if(buf[i]==' ') v.push_back(x),x=0; //遇到空格，说明一个数字结束;
        else x=x*10+buf[i]-'0';
    }
    v.push_back(x); //最后一个数字;
    return v;
}

//将数字ｖａｌ从ｐｏｓ位置开始拼接到ｓ后;
void itoa(char s[],int val,int& pos){
    int x=0;
    while(val){ //将数字翻转;
        x=x*10+val%10;
        val/=10;
    }
    if(!x){ s[pos++]='0'; return ;}
    while(x){
        s[pos++]=x%10+'0';
        x/=10;
    }
}

//辅助进行安全性检测的结构体;
struct Safe{
    int Need[MAXROW][MAXCAL];       //需求矩阵;
    int Allocation[MAXROW][MAXCAL]; //已分配资源矩阵;
    bool Finish[MAXROW];            //进程是否已回收资源;
    int Work[MAXCAL];               //系统可以分配出去的资源向量;
    int num;                        //已完成分配的进程数量;
    void init(){  //初始化;
        //memcpy(this,from,size); 将from数组按照size大小复制给this数组;
        memcpy(this->Need,Need,sizeof(Need));
        memcpy(this->Allocation,Allocation,sizeof(Allocation));
        memcpy(Work,Available,sizeof(Available));
        num=0;
        //赋值;
        memset(Finish,0,sizeof(Finish));
    }
    bool issafe(){ //是否安全;
        while(1){
            bool f=0; //判断是否存在一个可分配进程;

            for(int i=1;i<=n;++i){
                if(Finish[i]) continue; //该进程已被分配过;
                bool flag=1; //标记位记录是否存在一个可分配进程;

                for(int j=1;j<=m;++j)
                    if(Need[i][j]>Work[j]) flag=0; //某几项资源超出了系统当前拥有资源，不可分配;
                if(flag){
                    Finish[i]=1;
                    ++num;
                    for(int j=1;j<=m;++j) Work[j]+=Allocation[i][j]+Need[i][j];
                    f=1;
                }
            }
            if(!f) break; //没有找到可以分配的进程;
        }
        return num==n;
    }

}safe;
bool issafe(){
    safe.init();
    return safe.issafe();
}

//银行家算法;
/*
return -1 //请求资源数量超出进程自身所需;
        0 //当前无足够资源分配;
        1 //可分配且安全;
       -2 //不安全;
*/

int check(int Request[],int id){ //编号为ｉｄ的进程请求Ｒｅｑｕｅｓｔ资源;
    for(int i=1;i<=m;++i)
        if(Request[i]>Need[id][i]) return -1; //请求资源数量超出进程自身所需;
    for(int i=1;i<=m;++i)
        if(Request[i]>Available[i]) return 0; //当前无足够资源分配;
    for(int i=1;i<=m;++i){ //分配资源;
        Allocation[id][i]+=Request[i];
        Available[i]-=Request[i];
        Need[id][i]-=Request[i];
    }
    if(issafe()){ //可分配且安全

        return 1;
    }
    for(int i=1;i<=m;++i){ //回置分配前的状态;
        Allocation[id][i]-=Request[i];
        Available[i]+=Request[i];
        Need[id][i]+=Request[i];
    }
    return -2; //不安全;
}

//输出进程请求后的信息;
void output_check(int i){
    if(i==-1) cout<<"请求资源数量超出进程自身所需"<<endl;
    else if(i==0) cout<<"当前无足够资源分配"<<endl;
    else if(i==1) cout<<"可分配且安全"<<endl;
    else if(i==-2) cout<<"不安全"<<endl;
}

//打印当前所有进程的资源矩阵;
void pri(){
    cout<<"----------------------------"<<endl;
    printf("ID    ");
    /*------营造中间效果-------*/
    int mid=3*m/2+1;
    for(int i=1;i<=mid-2;++i) printf(" ");
    printf("Max");
    for(int i=mid+2;i<=3*m;++i) printf(" ");
    cout<<"     ";
    for(int i=1;i<=mid-2;++i) printf(" ");
    printf("Alc");
    for(int i=mid+2;i<=3*m;++i) printf(" ");
    cout<<"     ";
    printf("Need\n");
    /*------营造中间效果-------*/
    for(int i=1;i<=n;++i){
        if(ID[i]==0) continue; //跳过已分配完成的进程;
        printf("%d ",ID[i]);
        for(int j=1;j<=m;++j) printf("%2d ",Max[i][j]);
        cout<<"     ";
        for(int j=1;j<=m;++j) printf("%2d ",Allocation[i][j]);
        cout<<"     ";
        for(int j=1;j<=m;++j) printf("%2d ",Need[i][j]);
        cout<<endl;

    }
    cout<<"----------------------------"<<endl;
    cout<<endl;
    cout<<"Available: ";
    for(int i=1;i<=m;++i) cout<<Available[i]<<" ";
    cout<<endl;
    cout<<"----------------------------"<<endl;
    cout<<endl;
}

// 联合体，用于semctl初始化
union semun{
    int val; /*for SETVAL*/
    struct semid_ds *buf;
    unsigned short  *array;
};

// 初始化信号量
int init_sem(int sem_id, int value){
    union semun tmp;
    tmp.val = value;
    if(semctl(sem_id, 0, SETVAL, tmp) == -1){
        perror("Init Semaphore Error");
        return -1;
    }
    return 0;
}

// P操作:
//    若信号量值为1，获取资源并将信号量值-1
//    若信号量值为0，进程挂起等待
int sem_p(int sem_id){
    struct sembuf sbuf;
    sbuf.sem_num = 0; /*序号*/
    sbuf.sem_op = -1; /*P操作*/
    sbuf.sem_flg = SEM_UNDO;
    //semop(sem_id, &sbuf, 1);
    if(semop(sem_id, &sbuf, 1) == -1){
        perror("P operation Error");
        return -1;
    }
    return 0;
}

// V操作：
//    释放资源并将信号量值+1
//    如果有进程正在挂起等待，则唤醒它们
int sem_v(int sem_id){
    struct sembuf sbuf;
    sbuf.sem_num = 0; /*序号*/
    sbuf.sem_op = 1;  /*V操作*/
    sbuf.sem_flg = SEM_UNDO;
    //semop(sem_id, &sbuf, 1);
    if(semop(sem_id, &sbuf, 1) == -1){
        perror("V operation Error");
        return -1;
    }
    return 0;
}

// 删除信号量集;
int del_sem(int sem_id){
    union semun tmp;
    if(semctl(sem_id, 0, IPC_RMID, tmp) == -1){
        perror("Delete Semaphore Error");
        return -1;
    }
    return 0;
}

//输出每次请求的进程所请求的资源;
void output_request(vector<int> v){
    printf("%d进程请求的资源为: ",ID[v[0]]);
    printf("(");
    for(int i=1;i<v.size();++i) printf(" %d,",v[i]);
    printf(")\n");
}

//检测是否有进程已经获得了所有资源可以结束掉了;
void check_process(){
    for(int i=1;i<=n;++i){
        if(ID[i]==0) continue; //已结束的进程;
        bool f=0;
        for(int j=1;j<=m;++j)
            if(Need[i][j]) f=1;
        if(!f){ //该进程已不需要任何资源可以结束了;

            kill(ID[i],SIGKILL);
            waitpid(ID[i],NULL,0);
            ID[i]=0;
            --num;
            for(int j=1;j<=m;++j) Available[j]+=Allocation[i][j]; //回收资源;
        }
    }

}

//父进程的工作;
void parant_work(){
    vector<int> v=getval();
    int son_id=v[0]; // 标志第几个子进程;
    if(ID[son_id]){ //检查该进程是否存活;
        output_request(v);
        for(int i=1;i<v.size();++i) Request[i]=v[i];
        int res=check(Request,son_id); //执行银行家检测;
        output_check(res);
        pri();
        printf("是否杀死%d进程 y or n : ",ID[son_id]);
        char yorn[9];
        scanf("%s",yorn);
        system("clear");
        if(yorn[0]=='y'){
            kill(ID[son_id],SIGKILL); //通过信号的方式杀死子进程;
            --num; //存活进程数减一;
            waitpid(ID[son_id],NULL,0);//等待指定进程死亡，回收系统资源;
            printf("%d进程已死亡\n",ID[son_id]);
            ID[son_id]=0;
            for(int j=1;j<=m;++j) Available[j]+=Allocation[son_id][j]; //回收资源;
        }
    }
}
int main(){
    int sem_id; //信号量集id;
    key_t key;
    if((key = ftok(".", 'z')) < 0){ // 获取key值;
        perror("ftok error");
        exit(1);
    }
    if((sem_id = semget(key, 1, IPC_CREAT|0666)) == -1){ // 创建信号量集，其中只有一个信号量;
        perror("semget error");
        exit(1);
    }

    init_sem(sem_id, 1); // 初始化：初值设为0资源被占用;

    pid_t x; //进程ｉｄ;
    int fd[2];//管道通信的两个端口;

    int ret = pipe(fd);//生成一个管道;
    if(ret==-1){ //管道生成失败，报错误信息;
        perror("管道生成失败");
        exit(1);
    }
    srand((unsigned)time(0));//随机函数播种;
    num=n=rand()%6+1;            //生成进程数量;
    //n=num=2;
    m=rand()%6+1;            //生成资源数量;
    cout<<n<<" "<<m<<endl;
    for(int num=1;num<=n;++num){   //循环生成ｎ个子进程，形成一个扇形进程区;
        x=fork();
        if(x==0){  //ｆｏｒｋ函数会返回两次值，为父进程返回子进程ｉｄ，为子进程返回０，这里代表是一个子进程;
            id=num; //记录当前进程的编号;
            thisid=getpid();//记录当前子进程的pid;
            break;
        }
        ID[num]=x; //父进程记录所有子进程的ｉｄ;
    }

    if(x){ //父进程;
        for(int i=1;i<=m;++i) Available[i]=rand()%15;
        for(int i=1;i<=n;++i){
            for(int j=1;j<=m;++j) Max[i][j]=rand()%10+5;

            for(int j=1;j<=m;++j) Allocation[i][j]=rand()%Max[i][j];

            for(int j=1;j<=m;++j) Need[i][j]=Max[i][j]-Allocation[i][j];
        }
        cout<<"------------start------------"<<endl;
        pri();
        cout<<"-----------------------------"<<endl;
        while(1){
            sleep(1);
            if(!num) break; //所有进程均已结束;
            close(fd[1]);
            read(fd[0],buf,sizeof(buf));
            debug(buf)<<endl;
            parant_work();
            check_process();
            sem_v(sem_id);
        }
        del_sem(sem_id);
    }
    else{ //子进程;
        while(1){ //一直申请访问资源，直到父进程将其杀死;
            //printf("this is %d \n",getpid());
            sem_p(sem_id);    //P操作;
            for(int i=1;i<=m;++i) Request[i]=rand()%10;  //申请资源;
            close(fd[0]);  //关掉子进程读的管道;
            int pos=0;
            itoa(buf,id,pos);
            for(int i=1;i<=m;++i) buf[pos++]=' ',itoa(buf,Request[i],pos); //将请求向量组转化为字符串形式用以发送;
            write(fd[1],buf,strlen(buf)+1);  //写数据;
        }
    }

    return 0;
}
