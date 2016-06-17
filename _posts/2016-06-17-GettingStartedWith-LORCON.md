---
layout: post
title: GettingStartedWith-LORCON
date: 2016-06-17 21:58:37
tags: wireless
category: wireless
comments: true
---


reposted from: [http://blog.opensecurityresearch.com/2012/09/getting-started-with-lorcon.html](http://blog.opensecurityresearch.com/2012/09/getting-started-with-lorcon.html)


<h3 class='post-title entry-title' itemprop='name'>
Getting Started With LORCON
</h3>
<div class='post-header'>
<div class='post-header-line-1'></div>
</div>
<div class='post-body entry-content' id='post-body-8473888026105423743' itemprop='description articleBody'>
By Brad Antoniewicz.<BR><BR>

<b>L</b>oss <b>O</b>f <b>R</b>adio <b>CON</b>nectivity (LORCON) is an IEEE 802.11 packet injection library. It was originally created by <a href="http://www.willhackforsushi.com/">Joshua Wright</a> and <a href="http://www.kismetwireless.net/">Michael Kershaw ("dragorn")</a> - I think Johnny Cache was an early contributor as well. As of now, dragorn maintains it, however it doesn't seem that there have been many updates in the last year or so.<BR><BR>

One of the biggest issues in wireless tool development was that tools needed to be driver-specific, so if the author didn't take into account a specific driver, the tool didn't work. Additionally, many tools implemented their own functions for packet capture and injection, resulting in lots of code duplication. These issues were first brought up in a talk called <a href="http://packetfactory.openwall.net/projects/libradiate/802.11_toolkit-2.0.pdf">"The Need for an 802.11 Wireless Toolkit"</a> by Mike Schiffman at Blackhat in 2002. Schiffman released a proof of concept library called "libradiate" which offered a solution to these problems. Unfortunately libradiate fell off the edge of the earth and wireless hackers everywhere found themselves in a deep void surrounded by sadness. A few years later, in 2007, <a href="http://www.willhackforsushi.com/presentations/shmoocon2007.pdf">LORCON emerged</a>. It eased development issues by creating standard function calls for injection and capture, and added a layer of abstraction such that tool developers wouldn't need to worry about the wireless driver or adapter in use. <BR><BR>

There have been two major releases: LORCON (defunct) and LORCON2 (current). We'll use the terms LORCON and LORCON2 interchangeably throughout this post when referring to the current version. LORCON2 supports the Linux <a href="http://wireless.kernel.org/en/developers/Documentation/mac80211">mac80211 wireless drivers</a>. The release also includes a Ruby extension to facilitate Ruby development.<BR><BR>

Although LORCON hasn't been updated all to often, it still works well, is extremely powerful, and is very easy to use. For whatever reason, people seem to have forgotten about it, so this post will hopefully kick everyone in the butt and provide a quick intro into using the library. <BR><BR>

<h1>Installation and Setup</h1>
As usual, I'll be using BackTrack (BT5R3 to be specific) as my Linux distribution, and I've created a couple of code examples and patches which can be found on github repo here:<BR>

<ul><li><a href="https://github.com/OpenSecurityResearch/lorcon_examples">https://github.com/OpenSecurityResearch/lorcon_examples</a></li></ul><BR><BR>

The first step is to clone the the repo:<BR>

<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> root@bt:~# git clone https://github.com/OpenSecurityResearch/lorcon_examples.git

</code></pre><br><BR>

<h2>Core Installation</h2>

LORCON has supposedly been moved from its original location (<a href="http://802.11ninja.net/">http://802.11ninja.net/</a>) to a fancy new Google Code repo (<a href="https://code.google.com/p/lorcon/">https://code.google.com/p/lorcon/</a>), however I noticed the Google Code version doesn't include the packet forging functionality that I think is really useful, so we'll stick to the older release for the sake of this article. <BR><BR>

I made a couple of small changes to make the packet forging functionality work and enable a couple of other useful functions. The handful of changes are summed up in <a href="https://raw.github.com/OpenSecurityResearch/lorcon_examples/master/lorcon.patch">this patch</a>. Overall, the installation is pretty basic:<BR><br>

<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> root@bt:~# apt-get update
 root@bt:~# apt-get upgrade
 root@bt:~# apt-get install libnl-dev
 root@bt:~# svn co http://802.11ninja.net/svn/lorcon/trunk lorcon2
 root@bt:~# cd lorcon2
 root@bt:~/lorcon2# patch -p1 < ../lorcon_examples/lorcon.patch
 root@bt:~/lorcon2# ./configure --prefix=/usr
 root@bt:~/lorcon2# make depend
 root@bt:~/lorcon2# make
 root@bt:~/lorcon2# make install

</code></pre><br><BR>

<h2>Ruby Installation</h2>

In a <a href="http://blog.opensecurityresearch.com/2012/05/installing-lorcon2-on-backtrack-5-r2.html">previous blog post</a> Robert Portvliet detailed how to handle LORCON's Ruby extensions on BT5R2, specifically for Metasploit integration. That article details most of the specifics, so I'll just repeat the major points here for completeness. Note that the <a href="https://raw.github.com/OpenSecurityResearch/lorcon_examples/master/lorcon.patch"><code>lorcon.patch</code></a> includes the <code>STR2CSTR()</code> fixes. <BR><BR>

To install:<BR><BR>

<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> root@bt:~# cd lorcon2/ruby-lorcon/
 root@bt:~/lorcon2/ruby-lorcon# ruby extconf.rb
 root@bt:~/lorcon2/ruby-lorcon# make
 root@bt:~/lorcon2/ruby-lorcon# make install

</code></pre><br><BR>

Then to test:<BR><BR>


<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> root@bt:~/lorcon2/ruby-lorcon# ruby test.rb wlan1
Checking LORCON version
20091101

Fetching LORCON driver list
{"madwifing"=>
  {"name"=>"madwifing",
   "description"=>"Linux madwifi-ng drivers, deprecated by ath5k and ath9k"},
 "tuntap"=>
  {"name"=>"tuntap", "description"=>"Linux tuntap virtual interface drivers"},
 "mac80211"=>
  {"name"=>"mac80211",
   "description"=>
    "Linux mac80211 kernel drivers, includes all in-kernel drivers on modern systems"}}

Resolving driver by name 'mac80211'
{"name"=>"mac80211",
 "description"=>
  "Linux mac80211 kernel drivers, includes all in-kernel drivers on modern systems"}

Auto-detecting driver for interface wlan0
{"name"=>"mac80211",
 "description"=>
  "Linux mac80211 kernel drivers, includes all in-kernel drivers on modern systems"}

Created LORCON context

Opened as INJMON: wlan1mon

Channel: 11
#&lt;Lorcon::Packet:0x9e248a8&gt;
#&lt;Lorcon::Packet:0x9e24894&gt;

</code></pre><br><BR>

<h2>Python Bindings</h2>

There are actually two projects that extend LORCON into the Python world: <BR><br>
<ul>
<li><code>pylorcon</code> - Based on its <a href="http://code.google.com/p/pylorcon/source/list">change history</a>, this project was the first, starting in 2007. <code>pylorcon</code> started out with a <a href="http://code.google.com/p/pylorcon/">Google Code page</a> which now redirects users to a <a href="https://github.com/tom5760/pylorcon2">GitHub repository</a>. Looking at its source, the Google Code page appears to be written to support LORCON1 while the github page supports LORCON2. The GitHub repository is titled "<code>pylorcon2</code>" which confusingly enough, is the same name as the second project that extends LORCON2 to python.</li><BR>
<li><code>pylorcon2</code> - <a href="http://code.google.com/p/pylorcon2/source/list">In 2010</a> a couple of guys from Core Security created <code>pylorcon2</code>. This project consists mainly of a <a href="http://code.google.com/p/pylorcon2/">Google Code Page</a>. It supports all of the basic functionality of LORCON2, but doesn't support advanced functions such as capture loops.</li></ul><BR><BR>

For the purpose of this article, I'll use the <a href="http://code.google.com/p/pylorcon2/">Google Code <code>pylorcon2</code></a>. Although the <a href="https://github.com/tom5760/pylorcon2">GitHub <code>pylorcon2</code></a> actually has more functionality, I stumbled upon the Google Code one first and so I've grown more comfortable with it. If you have a good reason to choose one over the other, please let me know in the comments below.<br><BR>

To install:<BR><BR>

<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> root@bt:~# apt-get update
 root@bt:~# apt-get upgrade
 root@bt:~# apt-get install libnl-dev
 root@bt:~# wget http://pylorcon2.googlecode.com/files/PyLorcon2-0.1.tar.gz
 root@bt:~# tar -zxvf PyLorcon2-0.1.tar.gz
 root@bt:~# cd PyLorcon2-0.1
 root@bt:~/PyLorcon2-0.1# python setup.py build
 root@bt:~/PyLorcon2-0.1# python setup.py install

</code></pre><br><BR>

If you try to use the <code>test.py</code> of the Google Code <code>pylorcon2</code> it'll fail when trying to set the MAC address. Everything else works fine though, so you can either comment out the <code>def testMAC(self)</code> function or just ignore it. I haven't spent any time tracking down the root cause of the issue since it doesn't impact me much.<BR><BR>

To test (with <code>def testMAC(self)</code> commented out):

<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> root@bt:~/PyLorcon2-0.1# python test.py
testAutoDriver (__main__.PyLorcon2TestCase) ... ok
testChannel (__main__.PyLorcon2TestCase) ... ok
testFindDriver (__main__.PyLorcon2TestCase) ... ok
testGetDriverName (__main__.PyLorcon2TestCase) ... ok
testGetVersion (__main__.PyLorcon2TestCase) ... ok
testInjection (__main__.PyLorcon2TestCase) ... ok
testListDrivers (__main__.PyLorcon2TestCase) ... ok
testTimeout (__main__.PyLorcon2TestCase) ... ok
testVap (__main__.PyLorcon2TestCase) ... ok

----------------------------------------------------------------------
Ran 9 tests in 0.011s

OK

</code></pre><br><BR>

<h1>Program Structure</h1>

The sample code I created within the <a href="https://github.com/OpenSecurityResearch/lorcon_examples/">lorcon_examples</a> GitHub provides a couple of simple examples to get you started with LORCON. The code is broken up into blocks so that you can use them as templates to make your own packet injection tools. These examples all follow a basic structure, breaking the program up into three main blocks: <BR><BR>
<ol>
<li>Context setup</li>
<li>Injection/Capture</li>
<li>Context cleanup</li>
</ol>

<h2>Context setup</h2>

At the heart of LORCON is the "LORCON context" which is more or less representative of the wireless interface you're interacting with. The context adds a layer of abstraction between your program and the wireless driver. Instead of dealing with the driver directly, you deal with the context. This way your program doesn't need to worry about any driver specifics and thus can work with any drivers LORCON supports. <BR><BR>

The context needs to be configured though. First LORCON needs to figure out what driver is being used. The <code>lorcon_auto_driver()</code> function automatically determines this based on the interface provided. Here's some example C code that determines the driver for a provided interface:<BR>


<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> // Automatically determine the driver of the interface
        if ( (driver = lorcon_auto_driver(interface)) == NULL) {
                printf("[!] Could not determine the driver for %s\n",interface);
                return -1;
        } else {
                printf("[+]\t Driver: %s\n",driver->name);
        }

</code></pre><br><BR>

Now that we've determined the driver, we can set up the LORCON context. In C, the <code>lorcon_create()</code> function handles that for us:<BR>
<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> // Create LORCON context
        if ((context = lorcon_create(interface, driver)) == NULL) {
                printf("[!]\t Failed to create context");
                return -1;
        }

</code></pre><br><BR>

Next we'll need to enable monitor mode on the interface using the <code>lorcon_open_injmon()</code> function. This creates a <a href="http://wireless.kernel.org/en/users/Documentation/modes#Monitor_.28MON.29_mode"> Monitor Mode VAP</a> on the interface provided to handle monitoring and injection. <BR>

<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> // Create Monitor Mode Interface
        if (lorcon_open_injmon(context) < 0) {
                printf("[!]\t Could not create Monitor Mode interface!\n");
                return -1;
        } else {
                printf("[+]\t Monitor Mode VAP: %s\n",lorcon_get_vap(context));
                lorcon_free_driver_list(driver);
        }

</code></pre><br><BR>

Finally, we'll just need to set a specific channel to listen/transmit on with the <code>lorcon_set_channel</code>:<BR>

<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> // Set the channel we'll be injecting on
        lorcon_set_channel(context, channel);
        printf("[+]\t Using channel: %d\n\n",channel);

</code></pre><br><BR>

<h2>Injection/Capture</h2>

We'll focus specifically on injection for this article. Injection is handled with the <code>lorcon_send_bytes()</code> and <code>lorcon_inject()</code> functions. <code>lorcon_send_bytes()</code> is as basic as it gets, taking in a array of bytes and simply sending it:<BR>

<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> // Raw packet bytes (from capture_example.c included within LORCON)
        unsigned char packet[115] = {
        0x80, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // dur ffff
        0xff, 0xff, 0x00, 0x0f, 0x66, 0xe3, 0xe4, 0x03,
        0x00, 0x0f, 0x66, 0xe3, 0xe4, 0x03, 0x00, 0x00, // 0x0000 - seq no.
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // BSS timestamp
        0x64, 0x00, 0x11, 0x00, 0x00, 0x0f, 0x73, 0x6f,
        0x6d, 0x65, 0x74, 0x68, 0x69, 0x6e, 0x67, 0x63,
        0x6c, 0x65, 0x76, 0x65, 0x72, 0x01, 0x08, 0x82,
        0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, 0x03,
        0x01, 0x01, 0x05, 0x04, 0x00, 0x01, 0x00, 0x00,
        0x2a, 0x01, 0x05, 0x2f, 0x01, 0x05, 0x32, 0x04,
        0x0c, 0x12, 0x18, 0x60, 0xdd, 0x05, 0x00, 0x10,
        0x18, 0x01, 0x01, 0xdd, 0x16, 0x00, 0x50, 0xf2,
        0x01, 0x01, 0x00, 0x00, 0x50, 0xf2, 0x02, 0x01,
        0x00, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x00, 0x00,
        0x50, 0xf2, 0x02};

 // Send and exit if error
                if ( lorcon_send_bytes(context, sizeof(packet), packet) < 0 )
                        return -1;


</code></pre><br><BR>

The <code>lorcon_inject()</code> function requires the use of a <code>lorcon_packet_t</code> structure which contains the raw bytes plus a bunch of other information about the packet you're injecting. This comes in handy when you use LORCON's built-in packet forging capabilities which we'll talk about later on.<BR><BR>

<h2>Context cleanup</h2>

Finally, once our program has completed, we'll just need to close the interface we've created, and free up the context using the <code>lorcon_close()</code> and <code>lorcon_free()</code> functions:

<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> // Close the interface
        lorcon_close(context);

 // Free the LORCON Context
        lorcon_free(context);

</code></pre><br><BR>

<h1>Your First Flooder</h1>

Using the examples, lets see what a basic beacon flooder looks like using LORCON. <BR><br>

<h2>Using C</h2>

<a href="https://raw.github.com/OpenSecurityResearch/lorcon_examples/master/beacon_flood_raw.c"><code>beacon_flood_raw.c</code></a> is broken up into the three main parts as described above. It leverages the <code>lorcon_send_bytes()</code> function to send the packet defined in the <code>packet[]</code> array. I've added a slight delay in between sending packets, and also some friendly output for the user.<BR><BR>

To compile:<BR>
<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> root@bt:~/lorcon_examples# gcc -o beacon_flood_raw -lorcon2 beacon_flood_raw.c

</code></pre><br><BR>


To run:<BR>
<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> root@bt:~/lorcon_examples# ./beacon_flood_raw -i wlan1 -c 11

</code></pre><br><BR>

<h2>Using Python</h2>
<a href="https://raw.github.com/OpenSecurityResearch/lorcon_examples/master/beacon_flood_raw.py"><code>beacon_flood_raw.py</code></a> is pretty much the minor image of <code>beacon_flood_raw.c</code> but in Python - same output and all.<BR><BR>

To run:<BR>
<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> root@bt:~/lorcon_examples# python beacon_flood_raw.py -i wlan1 -c 11

</code></pre><br><BR>


<h1>Using LORCON for Packet Creation</h1>

One feature that I absolutely love is the LORCON's packet forging capabilities. Rather than using a byte array for your frame, LORCON allows you to create a frame on the fly within a structure called a <code>metapack</code>. Once you've built your frame, you transform it into a <code>lorcon_packet_t</code> then send it using <code>lorcon_inject()</code>. There are a bunch of functions to create any type of frame you'd like:<BR>
<ul>
<li><code>lcpf_80211headers()</code></li>
<li><code>lcpf_80211ctrlheaders()</code></li>
<li><code>lcpf_qosheaders()</code></li>
<li><code>lcpf_beacon()</code></li>
<li><code>lcpf_add_ie()</code></li>
<li><code>lcpf_disassoc()</code></li>
<li><code>lcpf_probereq()</code></li>
<li><code>lcpf_proberesp()</code></li>
<li><code>lcpf_rts()</code></li>
<li><code>lcpf_deauth()</code></li>
<li><code>lcpf_authreq(()</code></li>
<li><code>lcpf_authresp()</code></li>
<li><code>lcpf_assocreq()</code></li>
<li><code>lcpf_assocresp()</code></li>
<li><code>lcpf_data()</code></li>
</ul><BR>

Following our previous examples, lets create a beacon frame with <code>lcpf_beacon()</code> and add various Information Element(IE) tags to it with <code>lcpf_add_ie()</code>:<BR>

<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> lcpa_metapack_t *metapack; // metapack for LORCON packet assembly
 lorcon_packet_t *txpack; // The raw packet to be sent

 // Initialize the LORCON metapack
 metapack = lcpa_init();

 // Create a Beacon frame from 00:DE:AD:BE:EF:00
 lcpf_beacon(metapack, mac, mac, 0x00, 0x00, 0x00, 0x00, timestamp, interval, capabilities);

 // Append IE Tag 0 for SSID
 lcpf_add_ie(metapack, 0, strlen(ssid),ssid);

 // Most of the following IE tags are not needed, but added here as examples

 // Append IE Tag 1 for rates
 lcpf_add_ie(metapack, 1, sizeof(rates)-1, rates);

 // Append IE Tag 3 for Channel
 lcpf_add_ie(metapack, 3, 1, &channel);

 // Append IE Tags 42/47 for ERP Info
 lcpf_add_ie(metapack, 42, 1, "\x05");
 lcpf_add_ie(metapack, 47, 1, "\x05");

</code></pre><br><BR>

Next we'll convert it to a LORCON packet (<code>lorcon_packet_t</code>):<BR>

<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> // Convert the LORCON metapack to a LORCON packet for sending
 txpack = (lorcon_packet_t *) lorcon_packet_from_lcpa(context, metapack);

</code></pre><br><BR>

and send:<BR>

<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> // Send and exit if error
 if ( lorcon_inject(context,txpack) < 0 )
            return -1;

</code></pre><br><BR>

To put the entire picture together, this is all shown in <a href="https://raw.github.com/OpenSecurityResearch/lorcon_examples/master/beacon_flood_lcpa.c"><code>beacon_flood_lcpa.c</code></a>. <BR><BR>

To compile:<BR>
<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> root@bt:~/lorcon_examples# gcc -o beacon_flood_lcpa -lorcon2 beacon_flood_lcpa.c

</code></pre><br><BR>


To run:<BR>
<pre  style="font-family:arial;font-size:12px;border:1px dashed #CCCCCC;width:99%;height:auto;overflow:auto;background:#f0f0f0;padding:0px;color:#000000;text-align:left;line-height:20px;"><code style="color:#000000;word-wrap:normal;"> root@bt:~/lorcon_examples# ./beacon_flood_lcpa -s brad -i wlan1 -c 11

</code></pre><br><BR>

<h1>Enjoy!</h1>

Now its your turn - try to take one of the examples and create a tool to de-authenticate users from an access point!<BR>