#include <mutex>
#include <condition_variable>
#include <deque>
#include <vector>
#include <string>
#include <thread>
std::mutex mx;
std::condition_variable cv;
std::deque<std::shared_ptr<std::pair<int,std::string> > > dq_str;
bool stop_flag=false;

void worker(const std::string searchTemplate)
{
    while(1)
    {
        /// wait block, wait until thread be notified
        {
            std::unique_lock<std::mutex> g(mx);
            if(stop_flag && dq_str.empty())
                return;
            cv.wait(g);

        }
        /// after wait process all items
        while(dq_str.size())
        {
            std::shared_ptr<std::pair<int,std::string> > str;
            {
                std::unique_lock<std::mutex> g(mx);
                if(stop_flag && dq_str.empty())
                    return;
                if(dq_str.empty())
                    continue;

                str=dq_str[0];
                dq_str.pop_front();
            }
            if(str->second.size()>searchTemplate.size())
            {
                for(size_t i=0;i<str->second.size()-searchTemplate.size();i++)
                {
                    bool found=true;
                    for(size_t j=0;j<searchTemplate.size();j++)
                    {
                        char str_sym=str->second[i+j];
                        char tmpl_sym=searchTemplate[j];
                        if(!(tmpl_sym=='?' || str_sym==tmpl_sym))
                        {
                            found=false;
                            break;
                        }
                    }
                    if(found)
                    {
                        printf("%d %d %s\n",str->first,i,str->second.substr(i,searchTemplate.size()).c_str());
                    }
                }
            }
        }
    }
}
int main(int argc, char *argv[])
{

    if(argc!=3)
    {
        printf("usage: %s <filename> <searchstring>\n",argv[0]);
        return 1;
    }
    std::string fileName=argv[1];
    std::string searchTemplate=argv[2];
    std::vector<std::shared_ptr<std::thread> > threads;

    /// make 10 workers;
    for(int i=0;i<10;i++)
    {
        std::shared_ptr<std::thread> th1=std::make_shared<std::thread>(std::thread(worker,searchTemplate));
        threads.push_back(th1);
    }


    FILE * f_in=fopen(fileName.c_str(),"r");
    char buf[1024];
    int cnt=0;
    while(fgets(buf,sizeof(buf),f_in))
    {
        cnt++;
        bool need_run_thread=false;
        {
            std::unique_lock<std::mutex> lock(mx);

            if(threads.size()==0 || dq_str.size()/threads.size()>5)
            {
                need_run_thread=true;
            }
            std::shared_ptr<std::pair<int,std::string> > pp=std::make_shared<std::pair<int,std::string>> (cnt,buf);
            dq_str.push_back(pp);
            cv.notify_one();
        }
    }
    fclose(f_in);

    stop_flag=true;
    /// flushing workers, to finish work
    {
        cv.notify_all();


    }
    /// wait thread terminations
    for(auto& z: threads)
    {
        z->join();
    }
    printf("all done\n");
    return 0;
}
