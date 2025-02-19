


template<class F,class... Args>

auto exec(int64_t timeoutMs,F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
    int64_t expireTime = (timeoutMs ==0 ? 0 :TNOWMS + timeoutMs); 
    
    using RetType = decltype(f(args...));

    auto task = std::make_shared<std::packaged_task<RetType()>>(
        std::bind(std::forward<F>(f),std::forward<Args>(args)...)
    );

    TaskFuncPtr fPtr = std::make_shared<TaskFunc>(expireTime);

    fPtr->task = [task](){
        (*task)();
    };

    std::unique_lock<std::mutex> lock(mutex_);
    tasks_.push(fPtr);
    condition_.notify_one();

    return task->get_future();
}