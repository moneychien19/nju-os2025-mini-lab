# M3 - sperf

[Requirements](https://jyywiki.cn/OS/2025/labs/M3.md)

## Run

```bash
make
./sperf COMMAND [ARG]
```

## What I've Learned

Lab 的指示如下：
> 我们把重要的步骤为大家分解好：
> 1. 使用 fork 创建一个新的进程；
> 2. 子进程使用 execve 调用 strace COMMAND ARG...，启动一份 strace；注意在 execve 成功返回以后，子进程已经不再受控制了，strace 会不断输出系统调用的 trace，直到程序结束。程序不结束 strace 也不会结束；
> 3. 父进程想办法不断读取 strace 的输出，直到 strace 程序结束，读取到输出后，如果距离上次输出已经超过 100ms，就把统计信息打印到屏幕上。

- 為什麼我們需要用 `fork` 創建新的進程，而不是在當下的進程裡直接執行 `execve`？
  - 因為一旦執行 `execve` 就會進行復位，重置當下的進程，目前在跑的 C 程式就會原地消失，所以必須要創建新的進程，讓欲分析的程式在乾淨的進程中跑
- 我們要如何讓父進程讀取子進程的 strace 輸出結果？
  - `fork` 後子進程就會立即返回 pid 給父進程，因此父進程預設是不能隨時拿到子進程中的狀態的
  - 解法是在 `fork` 之前先建立一個 pipe，這樣父子進程就會共用 pipe 的同一組 fd，我們再將父進程的寫端關掉、子進程的讀端關掉，這樣子進程的輸出就可以被父進程讀到了
- 如何用 `strace` 取得執行時間？
  - 使用 `strace -T`，每一行最後面的 `<>` 中包含的數字即為執行時間
- 如何每 100ms 輸出一次？