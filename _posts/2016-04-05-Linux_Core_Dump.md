---
layout: post
title: Linux_Core_Dump
date: 2016-04-05 00:04:10
tags: gdb
category: gdb
comments: true
---


<p>当程序运行的过程中异常终止或崩溃，操作系统会将程序当时的内存状态记录下来，保存在一个文件中，这种行为就叫做<strong>Core Dump</strong>（中文有的翻译成&ldquo;核心转储&rdquo;)。我们可以认为 core dump 是&ldquo;内存快照&rdquo;，但实际上，除了内存信息之外，还有些关键的程序运行状态也会同时 dump 下来，例如寄存器信息（包括程序指针、栈指针等）、内存管理信息、其他处理器和操作系统状态和信息。core dump 对于编程人员诊断和调试程序是非常有帮助的，因为对于有些程序错误是很难重现的，例如指针异常，而 core dump 文件可以再现程序出错时的情景。</p>
<h2>Core Dump 名词解释</h2>
<p>在半导体作为电脑内存材料之前，电脑内存使用的是&nbsp;<a href="http://en.wikipedia.org/wiki/Core_memory"><strong>磁芯内存</strong></a>（Magnetic Core Memory），Core Dump 中的 Core 沿用了磁芯内存的&nbsp;<strong>Core</strong>&nbsp;表达。图为磁芯内存的一个单元，来自&nbsp;<a href="http://en.wikipedia.org/wiki/Main_Page">Wikipedia</a>.</p>
<p><a href="https://github-camo.global.ssl.fastly.net/ae6f7881a591553390396805e918c314e2b00ff1/687474703a2f2f75706c6f61642e77696b696d656469612e6f72672f77696b6970656469612f636f6d6d6f6e732f7468756d622f642f64612f4b4c5f436f72654d656d6f72792e6a70672f36303070782d4b4c5f436f72654d656d6f72792e6a7067" target="_blank"><img src="https://github-camo.global.ssl.fastly.net/ae6f7881a591553390396805e918c314e2b00ff1/687474703a2f2f75706c6f61642e77696b696d656469612e6f72672f77696b6970656469612f636f6d6d6f6e732f7468756d622f642f64612f4b4c5f436f72654d656d6f72792e6a70672f36303070782d4b4c5f436f72654d656d6f72792e6a7067" alt="" /></a></p>
<p>在&nbsp;<strong><em>APUE</em></strong>&nbsp;一书中作者有句话这样写的：</p>
<blockquote>
<p>Because the file is named&nbsp;<em>core</em>, it shows how long this feature has been part of the Unix System.</p>
</blockquote>
<p>这里的 core 就是沿用的是早期电脑磁芯内存中的表达，也能看出 Unix 系统 Core Dump 机制的悠久历史。</p>
<p><strong>Dump</strong>&nbsp;指的是拷贝一种存储介质中的部分内容到另一个存储介质，或者将内容打印、显示或者其它输出设备。dump 出来的内容是格式化的，可以使用一些工具来解析它。</p>
<p>现代操作系统中，用&nbsp;<strong>Core Dump</strong>&nbsp;表示当程序异常终止或崩溃时，将进程此时的内存中的内容拷贝到磁盘文件中存储，以方便编程人员调试。</p>
<h2>Core Dump 如何产生</h2>
<p>上面说当程序运行过程中<strong>异常终止</strong>或<strong>崩溃</strong>时会发生 core dump，但还没说到什么具体的情景程序会发生异常终止或崩溃，例如我们使用&nbsp;<code>kill -9</code>&nbsp;命令杀死一个进程会发生 core dump 吗？实验证明是不能的，那么什么情况会产生呢？</p>
<p>Linux 中信号是一种异步事件处理的机制，每种信号对应有其默认的操作，你可以在&nbsp;<strong><a href="http://man7.org/linux/man-pages/man7/signal.7.html">这里</a></strong>&nbsp;查看 Linux 系统提供的信号以及默认处理。默认操作主要包括忽略该信号（Ingore）、暂停进程（Stop）、终止进程（Terminate）、终止并发生core dump（core）等。如果我们信号均是采用默认操作，那么，以下列出几种信号，它们在发生时会产生 core dump:</p>
<table>
<thead>
<tr><th>Signal</th><th>Action</th><th>Comment</th></tr>
</thead>
<tbody>
<tr>
<td>SIGQUIT</td>
<td>Core</td>
<td>Quit from keyboard</td>
</tr>
<tr>
<td>SIGILL</td>
<td>Core</td>
<td>Illegal Instruction</td>
</tr>
<tr>
<td>SIGABRT</td>
<td>Core</td>
<td>Abort signal from&nbsp;<a href="http://man7.org/linux/man-pages/man3/abort.3.html">abort</a></td>
</tr>
<tr>
<td>SIGSEGV</td>
<td>Core</td>
<td>Invalid memory reference</td>
</tr>
<tr>
<td>SIGTRAP</td>
<td>Core</td>
<td>Trace/breakpoint trap</td>
</tr>
</tbody>
</table>
<p>当然不仅限于上面的几种信号。这就是为什么我们使用&nbsp;<code>Ctrl+z</code>&nbsp;来挂起一个进程或者&nbsp;<code>Ctrl+C</code>&nbsp;结束一个进程均不会产生 core dump，因为前者会向进程发出&nbsp;<strong>SIGTSTP</strong>&nbsp;信号，该信号的默认操作为暂停进程（Stop Process）；后者会向进程发出<strong>SIGINT</strong>&nbsp;信号，该信号默认操作为终止进程（Terminate Process）。同样上面提到的&nbsp;<code>kill -9</code>&nbsp;命令会发出&nbsp;<strong>SIGKILL</strong>&nbsp;命令，该命令默认为终止进程。而如果我们使用&nbsp;<code>Ctrl+\</code>&nbsp;来终止一个进程，会向进程发出&nbsp;<strong>SIGQUIT</strong>&nbsp;信号，默认是会产生 core dump 的。还有其它情景会产生 core dump， 如：程序调用&nbsp;<code>abort()</code>&nbsp;函数、访存错误、非法指令等等。</p>
<p>下面举两个例子来说明：</p>
<ul>
<li>
<p>终端下比较&nbsp;<code>Ctrl+C</code>&nbsp;和&nbsp;<code>Ctrl+\</code>：</p>
<div class="highlight highlight-bash">
<pre>guohailin@guohailin:~<span class="nv">$ </span>sleep 10        <span class="c">#使用sleep命令休眠 10 s</span>
^C                           <span class="c">#使用 Ctrl+C 终止该程序,不会产生 core dump</span>
guohailin@guohailin:~<span class="nv">$ </span>sleep 10
^<span class="se">\Q</span>uit <span class="o">(</span>core dumped<span class="o">)</span>                <span class="c">#使用 Ctrl+\ 退出程序, 会产生 core dump</span>
guohailin@guohailin:~<span class="nv">$ </span>ls         <span class="c">#多出下面一个 core 文件</span>
-rw------- &nbsp;1 guohailin guohailin 335872 10月 22 11:31 sleep.core.21990
</pre>
</div>
</li>
<li>
<p>小程序产生 core dump</p>
<div class="highlight highlight-c">
<pre><span class="cp">#include &lt;stdio.h&gt;</span>

<span class="kt">int</span> <span class="nf">main</span><span class="p">()</span>
<span class="p">{</span>
    <span class="kt">int</span> <span class="o">*</span><span class="n">null_ptr</span> <span class="o">=</span> <span class="nb">NULL</span><span class="p">;</span>
    <span class="o">*</span><span class="n">null_ptr</span> <span class="o">=</span> <span class="mi">10</span><span class="p">;</span>            <span class="c1">//对空指针指向的内存区域写,会发生段错误</span>
    <span class="k">return</span> <span class="mi">0</span><span class="p">;</span>
<span class="p">}</span>
</pre>
</div>
<div class="highlight highlight-bash">
<pre><span class="c">#编译执行</span>
guohailin@guohailin:~<span class="nv">$ </span>./a.out
Segmentation fault <span class="o">(</span>core dumped<span class="o">)</span>
guohailin@guohailin:~<span class="nv">$ </span>ls      <span class="c">#多出下面一个 core 文件</span>
-rw-------  1 guohailin guohailin 200704 10月 22 11:35 a.out.core.22070    
</pre>
</div>
</li>
</ul>
<h2>Linux 下打开 Core Dump</h2>
<p>我使用的 Linux 发行版是 Ubuntu 13.04，设置生成 core dump 文件的方法如下：</p>
<ul>
<li>
<p><strong>打开 core dump 功能</strong></p>
<ul>
<li>在终端中输入命令&nbsp;<code>ulimit -c</code>&nbsp;，输出的结果为 0，说明默认是关闭 core dump 的，即当程序异常终止时，也不会生成 core dump 文件。</li>
<li>我们可以使用命令&nbsp;<code>ulimit -c unlimited</code>&nbsp;来开启 core dump 功能，并且不限制 core dump 文件的大小； 如果需要限制文件的大小，将 unlimited 改成你想生成 core 文件最大的大小，注意单位为 blocks（KB）。</li>
<li>用上面命令只会对当前的终端环境有效，如果想需要永久生效，可以修改文件&nbsp;<code>/etc/security/limits.conf</code>文件，关于此文件的设置参看&nbsp;<strong><a href="http://manpages.ubuntu.com/manpages/hardy/man5/limits.conf.5.html">这里</a></strong>&nbsp;。增加一行:</li>
</ul>
<div class="highlight highlight-bash">
<pre><span class="c"># /etc/security/limits.conf</span>
<span class="c">#</span>
<span class="c">#Each line describes a limit for a user in the form:</span>
<span class="c">#</span>
<span class="c">#&lt;domain&gt;   &lt;type&gt;   &lt;item&gt;   &lt;value&gt;</span>
    *          soft     core   unlimited
</pre>
</div>
</li>
<li>
<p><strong>修改 core 文件保存的路径</strong></p>
<ul>
<li>默认生成的 core 文件保存在可执行文件所在的目录下，文件名就为&nbsp;<code>core</code>。</li>
<li>通过修改&nbsp;<code>/proc/sys/kernel/core_uses_pid</code>&nbsp;文件可以让生成 core 文件名是否自动加上 pid 号。<br />例如&nbsp;<code>echo 1 &gt; /proc/sys/kernel/core_uses_pid</code>&nbsp;，生成的 core 文件名将会变成&nbsp;<code>core.pid</code>，其中 pid 表示该进程的 PID。</li>
<li>还可以通过修改&nbsp;<code>/proc/sys/kernel/core_pattern</code>&nbsp;来控制生成 core 文件保存的位置以及文件名格式。<br />例如可以用&nbsp;<code>echo "/tmp/corefile-%e-%p-%t" &gt; /proc/sys/kernel/core_pattern</code>&nbsp;设置生成的 core 文件保存在 &ldquo;/tmp/corefile&rdquo; 目录下，文件名格式为 &ldquo;core-命令名-pid-时间戳&rdquo;。<strong><a href="http://man7.org/linux/man-pages/man5/core.5.html">这里</a></strong>&nbsp;有更多详细的说明！</li>


</ul>


</li>


</ul>
<h2>使用 gdb 调试 Core 文件</h2>
<p>产生了 core 文件，我们该如何使用该 Core 文件进行调试呢？Linux 中可以使用 GDB 来调试 core 文件，步骤如下：</p>
<ul>
<li>首先，使用 gcc 编译源文件，加上&nbsp;<code>-g</code>&nbsp;以增加调试信息；</li>
<li>按照上面打开 core dump 以使程序异常终止时能生成 core 文件；</li>
<li>运行程序，当core dump 之后，使用命令&nbsp;<code>gdb program core</code>&nbsp;来查看 core 文件，其中 program 为可执行程序名，core 为生成的 core 文件名。</li>


</ul>
<p>下面用一个简单的例子来说明：</p>
<div class="highlight highlight-c">
<pre><span class="cp">#include &lt;stdio.h&gt;</span>
<span class="kt">int</span> <span class="nf">func</span><span class="p">(</span><span class="kt">int</span> <span class="o">*</span><span class="n">p</span><span class="p">)</span>
<span class="p">{</span>
    <span class="kt">int</span> <span class="n">y</span> <span class="o">=</span> <span class="o">*</span><span class="n">p</span><span class="p">;</span>
    <span class="k">return</span> <span class="n">y</span><span class="p">;</span>
<span class="p">}</span>
<span class="kt">int</span> <span class="nf">main</span><span class="p">()</span>
<span class="p">{</span>
    <span class="kt">int</span> <span class="o">*</span><span class="n">p</span> <span class="o">=</span> <span class="nb">NULL</span><span class="p">;</span>
    <span class="k">return</span> <span class="n">func</span><span class="p">(</span><span class="n">p</span><span class="p">);</span>
<span class="p">}</span>
</pre>
</div>
<p>编译加上调试信息, 运行之后core dump, 使用 gdb 查看 core 文件.</p>
<div class="highlight highlight-bash">
<pre>guohailin@guohailin:~<span class="nv">$ </span>gcc core_demo.c -o core_demo -g
guohailin@guohailin:~<span class="nv">$ </span>./core_demo 
Segmentation fault <span class="o">(</span>core dumped<span class="o">)</span>

guohailin@guohailin:~<span class="nv">$ </span>gdb core_demo core_demo.core.24816
...
Core was generated by <span class="s1">'./core_demo'</span>.
Program terminated with signal 11, Segmentation fault.
<span class="c">#0  0x080483cd in func (p=0x0) at core_demo.c:5</span>
5       int <span class="nv">y</span> <span class="o">=</span> *p;
<span class="o">(</span>gdb<span class="o">)</span>  where
<span class="c">#0  0x080483cd in func (p=0x0) at core_demo.c:5</span>
<span class="c">#1  0x080483ef in main () at core_demo.c:12</span>
<span class="o">(</span>gdb<span class="o">)</span> info frame
Stack level 0, frame at 0xffd590a4:
 <span class="nv">eip</span> <span class="o">=</span> 0x80483cd in func <span class="o">(</span>core_demo.c:5<span class="o">)</span>; saved eip 0x80483ef
 called by frame at 0xffd590c0
 <span class="nb">source </span>language c.
 Arglist at 0xffd5909c, args: <span class="nv">p</span><span class="o">=</span>0x0
 Locals at 0xffd5909c, Previous frame<span class="err">'</span>s sp is 0xffd590a4
 Saved registers:
  ebp at 0xffd5909c, eip at 0xffd590a0
<span class="o">(</span>gdb<span class="o">)</span> 
</pre>
</div>
<p>从上面可以看出,我们可以还原 core_demo 执行时的场景,并使用&nbsp;<code>where</code>&nbsp;可以查看当前程序调用函数栈帧, 还可以使用 gdb 中的命令查看寄存器,变量等信息.</p>
<h2>参考资料</h2>
<ul>
<li><a href="http://whatis.techtarget.com/definition/core-dump">http://whatis.techtarget.com/definition/core-dump</a></li>
<li><a href="http://man7.org/linux/man-pages/man5/core.5.html">http://man7.org/linux/man-pages/man5/core.5.html</a></li>
<li><a href="http://en.wikipedia.org/wiki/Core_dump">http://en.wikipedia.org/wiki/Core_dump</a></li>
</ul>