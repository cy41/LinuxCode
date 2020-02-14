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
// �������ȡһ���ź����飺���ɹ������ź�����ID��ʧ�ܷ���-1
int semget(key_t key, int num_sems, int sem_flags);
// ���ź�������в������ı��ź�����ֵ���ɹ�����0��ʧ�ܷ���-1
int semop(int semid, struct sembuf semoparray[], size_t numops);
// �����ź����������Ϣ
int semctl(int semid, int sem_num, int cmd, ...);
*/


using namespace std;

#define debug(x) cout<<(#x)<<" : "<<(x)

#define MAXROW 100 //��������;
#define MAXCAL 100 //�������̿�������������Դ��;
#define MAXSIZE 200//�ܳ�ͨ���е������������ַ�������;


char buf[MAXSIZE]; //�ܳ�ͨ������;

int Max[MAXROW][MAXCAL];        //�������;
int Allocation[MAXROW][MAXCAL]; //�ѷ�����Դ;
int Need[MAXROW][MAXCAL];       //����;
pid_t ID[MAXROW];               //��¼�ӽ��̣��,����ɷ������ã�;
int num;                        //�����ӽ��̵�����;
pid_t thisid;                   //�ӽ��̵�pid;

int Available[MAXCAL];          //ӵ����Դ��;
int n,m;                        //����������Դ��;
int id;                         //��־��ǰ����Ϊ���Ž���;
int Request[MAXCAL];            //������Դ��;

/*------------����˵����--------------*/

/*
vector<int> getal();                 //���ַ������Ϊ��������;
void itoa(char s[],int val,int& pos);//�����֣����ӣ���λ�ÿ�ʼƴ�ӵ����;
bool issafe();                       //��ȫ�Լ��;
int check(int Request[],int id);     //���м��㷨;
void output_check(int i);            //���������������Ϣ;
void pri();                          //��ӡ��ǰ���н��̵���Դ����;
int init_sem(int sem_id, int value); // ��ʼ���ź���;
int sem_p(int sem_id);               // P����:
int sem_v(int sem_id);               // V����;
int del_sem(int sem_id);             // ɾ���ź�����;
void output_request(vector<int> v);  //���ÿ������Ľ������������Դ;
void check_process();                //����Ƿ��н����Ѿ������������Դ���Խ�������;
*/

/*------------����˵����--------------*/

//���ַ������Ϊ��������;
vector<int> getval(){
    int n=strlen(buf);
    int x=0;
    vector<int> v;
    v.clear();
    for(int i=0;i<n;++i){
        if(buf[i]==' ') v.push_back(x),x=0; //�����ո�˵��һ�����ֽ���;
        else x=x*10+buf[i]-'0';
    }
    v.push_back(x); //���һ������;
    return v;
}

//�����֣����ӣ���λ�ÿ�ʼƴ�ӵ����;
void itoa(char s[],int val,int& pos){
    int x=0;
    while(val){ //�����ַ�ת;
        x=x*10+val%10;
        val/=10;
    }
    if(!x){ s[pos++]='0'; return ;}
    while(x){
        s[pos++]=x%10+'0';
        x/=10;
    }
}

//�������а�ȫ�Լ��Ľṹ��;
struct Safe{
    int Need[MAXROW][MAXCAL];       //�������;
    int Allocation[MAXROW][MAXCAL]; //�ѷ�����Դ����;
    bool Finish[MAXROW];            //�����Ƿ��ѻ�����Դ;
    int Work[MAXCAL];               //ϵͳ���Է����ȥ����Դ����;
    int num;                        //����ɷ���Ľ�������;
    void init(){  //��ʼ��;
        //memcpy(this,from,size); ��from���鰴��size��С���Ƹ�this����;
        memcpy(this->Need,Need,sizeof(Need));
        memcpy(this->Allocation,Allocation,sizeof(Allocation));
        memcpy(Work,Available,sizeof(Available));
        num=0;
        //��ֵ;
        memset(Finish,0,sizeof(Finish));
    }
    bool issafe(){ //�Ƿ�ȫ;
        while(1){
            bool f=0; //�ж��Ƿ����һ���ɷ������;

            for(int i=1;i<=n;++i){
                if(Finish[i]) continue; //�ý����ѱ������;
                bool flag=1; //���λ��¼�Ƿ����һ���ɷ������;

                for(int j=1;j<=m;++j)
                    if(Need[i][j]>Work[j]) flag=0; //ĳ������Դ������ϵͳ��ǰӵ����Դ�����ɷ���;
                if(flag){
                    Finish[i]=1;
                    ++num;
                    for(int j=1;j<=m;++j) Work[j]+=Allocation[i][j]+Need[i][j];
                    f=1;
                }
            }
            if(!f) break; //û���ҵ����Է���Ľ���;
        }
        return num==n;
    }

}safe;
bool issafe(){
    safe.init();
    return safe.issafe();
}

//���м��㷨;
/*
return -1 //������Դ��������������������;
        0 //��ǰ���㹻��Դ����;
        1 //�ɷ����Ұ�ȫ;
       -2 //����ȫ;
*/

int check(int Request[],int id){ //���Ϊ���Ľ�������ң���������Դ;
    for(int i=1;i<=m;++i)
        if(Request[i]>Need[id][i]) return -1; //������Դ��������������������;
    for(int i=1;i<=m;++i)
        if(Request[i]>Available[i]) return 0; //��ǰ���㹻��Դ����;
    for(int i=1;i<=m;++i){ //������Դ;
        Allocation[id][i]+=Request[i];
        Available[i]-=Request[i];
        Need[id][i]-=Request[i];
    }
    if(issafe()){ //�ɷ����Ұ�ȫ

        return 1;
    }
    for(int i=1;i<=m;++i){ //���÷���ǰ��״̬;
        Allocation[id][i]-=Request[i];
        Available[i]+=Request[i];
        Need[id][i]+=Request[i];
    }
    return -2; //����ȫ;
}

//���������������Ϣ;
void output_check(int i){
    if(i==-1) cout<<"������Դ��������������������"<<endl;
    else if(i==0) cout<<"��ǰ���㹻��Դ����"<<endl;
    else if(i==1) cout<<"�ɷ����Ұ�ȫ"<<endl;
    else if(i==-2) cout<<"����ȫ"<<endl;
}

//��ӡ��ǰ���н��̵���Դ����;
void pri(){
    cout<<"----------------------------"<<endl;
    printf("ID    ");
    /*------Ӫ���м�Ч��-------*/
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
    /*------Ӫ���м�Ч��-------*/
    for(int i=1;i<=n;++i){
        if(ID[i]==0) continue; //�����ѷ�����ɵĽ���;
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

// �����壬����semctl��ʼ��
union semun{
    int val; /*for SETVAL*/
    struct semid_ds *buf;
    unsigned short  *array;
};

// ��ʼ���ź���
int init_sem(int sem_id, int value){
    union semun tmp;
    tmp.val = value;
    if(semctl(sem_id, 0, SETVAL, tmp) == -1){
        perror("Init Semaphore Error");
        return -1;
    }
    return 0;
}

// P����:
//    ���ź���ֵΪ1����ȡ��Դ�����ź���ֵ-1
//    ���ź���ֵΪ0�����̹���ȴ�
int sem_p(int sem_id){
    struct sembuf sbuf;
    sbuf.sem_num = 0; /*���*/
    sbuf.sem_op = -1; /*P����*/
    sbuf.sem_flg = SEM_UNDO;
    //semop(sem_id, &sbuf, 1);
    if(semop(sem_id, &sbuf, 1) == -1){
        perror("P operation Error");
        return -1;
    }
    return 0;
}

// V������
//    �ͷ���Դ�����ź���ֵ+1
//    ����н������ڹ���ȴ�����������
int sem_v(int sem_id){
    struct sembuf sbuf;
    sbuf.sem_num = 0; /*���*/
    sbuf.sem_op = 1;  /*V����*/
    sbuf.sem_flg = SEM_UNDO;
    //semop(sem_id, &sbuf, 1);
    if(semop(sem_id, &sbuf, 1) == -1){
        perror("V operation Error");
        return -1;
    }
    return 0;
}

// ɾ���ź�����;
int del_sem(int sem_id){
    union semun tmp;
    if(semctl(sem_id, 0, IPC_RMID, tmp) == -1){
        perror("Delete Semaphore Error");
        return -1;
    }
    return 0;
}

//���ÿ������Ľ������������Դ;
void output_request(vector<int> v){
    printf("%d�����������ԴΪ: ",ID[v[0]]);
    printf("(");
    for(int i=1;i<v.size();++i) printf(" %d,",v[i]);
    printf(")\n");
}

//����Ƿ��н����Ѿ������������Դ���Խ�������;
void check_process(){
    for(int i=1;i<=n;++i){
        if(ID[i]==0) continue; //�ѽ����Ľ���;
        bool f=0;
        for(int j=1;j<=m;++j)
            if(Need[i][j]) f=1;
        if(!f){ //�ý����Ѳ���Ҫ�κ���Դ���Խ�����;

            kill(ID[i],SIGKILL);
            waitpid(ID[i],NULL,0);
            ID[i]=0;
            --num;
            for(int j=1;j<=m;++j) Available[j]+=Allocation[i][j]; //������Դ;
        }
    }

}

//�����̵Ĺ���;
void parant_work(){
    vector<int> v=getval();
    int son_id=v[0]; // ��־�ڼ����ӽ���;
    if(ID[son_id]){ //���ý����Ƿ���;
        output_request(v);
        for(int i=1;i<v.size();++i) Request[i]=v[i];
        int res=check(Request,son_id); //ִ�����мҼ��;
        output_check(res);
        pri();
        printf("�Ƿ�ɱ��%d���� y or n : ",ID[son_id]);
        char yorn[9];
        scanf("%s",yorn);
        system("clear");
        if(yorn[0]=='y'){
            kill(ID[son_id],SIGKILL); //ͨ���źŵķ�ʽɱ���ӽ���;
            --num; //����������һ;
            waitpid(ID[son_id],NULL,0);//�ȴ�ָ����������������ϵͳ��Դ;
            printf("%d����������\n",ID[son_id]);
            ID[son_id]=0;
            for(int j=1;j<=m;++j) Available[j]+=Allocation[son_id][j]; //������Դ;
        }
    }
}
int main(){
    int sem_id; //�ź�����id;
    key_t key;
    if((key = ftok(".", 'z')) < 0){ // ��ȡkeyֵ;
        perror("ftok error");
        exit(1);
    }
    if((sem_id = semget(key, 1, IPC_CREAT|0666)) == -1){ // �����ź�����������ֻ��һ���ź���;
        perror("semget error");
        exit(1);
    }

    init_sem(sem_id, 1); // ��ʼ������ֵ��Ϊ0��Դ��ռ��;

    pid_t x; //���̣��;
    int fd[2];//�ܵ�ͨ�ŵ������˿�;

    int ret = pipe(fd);//����һ���ܵ�;
    if(ret==-1){ //�ܵ�����ʧ�ܣ���������Ϣ;
        perror("�ܵ�����ʧ��");
        exit(1);
    }
    srand((unsigned)time(0));//�����������;
    num=n=rand()%6+1;            //���ɽ�������;
    //n=num=2;
    m=rand()%6+1;            //������Դ����;
    cout<<n<<" "<<m<<endl;
    for(int num=1;num<=n;++num){   //ѭ�����ɣ���ӽ��̣��γ�һ�����ν�����;
        x=fork();
        if(x==0){  //����뺯���᷵������ֵ��Ϊ�����̷����ӽ��̣�䣬Ϊ�ӽ��̷��أ������������һ���ӽ���;
            id=num; //��¼��ǰ���̵ı��;
            thisid=getpid();//��¼��ǰ�ӽ��̵�pid;
            break;
        }
        ID[num]=x; //�����̼�¼�����ӽ��̵ģ��;
    }

    if(x){ //������;
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
            if(!num) break; //���н��̾��ѽ���;
            close(fd[1]);
            read(fd[0],buf,sizeof(buf));
            debug(buf)<<endl;
            parant_work();
            check_process();
            sem_v(sem_id);
        }
        del_sem(sem_id);
    }
    else{ //�ӽ���;
        while(1){ //һֱ���������Դ��ֱ�������̽���ɱ��;
            //printf("this is %d \n",getpid());
            sem_p(sem_id);    //P����;
            for(int i=1;i<=m;++i) Request[i]=rand()%10;  //������Դ;
            close(fd[0]);  //�ص��ӽ��̶��Ĺܵ�;
            int pos=0;
            itoa(buf,id,pos);
            for(int i=1;i<=m;++i) buf[pos++]=' ',itoa(buf,Request[i],pos); //������������ת��Ϊ�ַ�����ʽ���Է���;
            write(fd[1],buf,strlen(buf)+1);  //д����;
        }
    }

    return 0;
}
