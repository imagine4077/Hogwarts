---
layout: post
title: "Promiscuous vs. Monitor Mode (Wireless Context)"
date: 2016-06-25 17:51:41
tags: wireless
category: wireless
comments: true
---


reposted from: [http://lazysolutions.blogspot.com/2008/10/difference-promiscuous-vs-monitor-mode.html](http://lazysolutions.blogspot.com/2008/10/difference-promiscuous-vs-monitor-mode.html)

<h3 class="post-title entry-title" itemprop="name">
Difference - Promiscuous vs. Monitor Mode (Wireless Context)
</h3>
<div class="post-header">
<div class="post-header-line-1"></div>
</div>
<div class="post-body entry-content" id="post-body-2309522277270440936" itemprop="description articleBody">
Monitor mode (RFMON) enables a wireless nic to capture packets without associating with an access point or ad-hoc network. This is desireable in that you can choose to "monitor" a specific channel, and you need never transmit any packets. In fact transmiting is sometimes not possible while in monitor mode (driver dependent). Another aspect of monitor mode is that the NIC does not care whether the CRC values are correct for packets captured in monitor mode, so some packets that you see may in fact be corrupted. Monitor mode only applies to wireless networks, while promiscuous mode can be used on both wired and wireless networks. Monitor mode is one of the six modes that 802.11 wireless cards can operate in: Master (acting as an access point), Managed (client, also known as station), Ad-hoc, Mesh, Repeater, and Monitor mode.<br><br>Promiscuous mode allows you to view all wireless packets on a network to which you have associated. The need to associate means that you must have some measn of authenticating yourself with an access point. In promiscuous mode, you will not see packets until you have associated. Not all wireless drivers support promiscuous mode. <br><br>One important difference is Promiscuous mode does, indeed, tell the card to process all frames, (i.e. remove 802.11 frame headers ) including those not destined for it. <br>But, monitor mode tells the card to pass along the frames intact (with 802.11 headers) and not present plain 'ol Ethernet frames to the host.<br><br><strong>Reason</strong><br>In monitor mode the SSID filter mentioned above is disabed and all packets of all SSID's from the currently selected channel are captured.<br><br>Even in promiscuous mode, an 802.11 adapter will only supply packets to the host of the SSID the adapter has joined. Although it can receive, at the radio level, packets on other SSID's, it will not forward them to the host.<br><br>So in order to capture all traffic that the adapter can receive, the adapter must be put into "monitor mode", sometimes called "rfmon mode". In this mode, the driver will not make the adapter a member of any service set, so it won't support sending any traffic and will only supply received packets to a packet capture mechanism, not to the networking stack. This means that the machine will not be able to use that adapter for network traffic; if it doesn't have any other network adapters, it will not be able to:<br><br>    * resolve addresses to host names using a network protocol such as DNS;<br>    * save packets to a file on a network file server; <br><br>etc..<br><br>Monitor mode is not supported by WinPcap, and thus not by Wireshark or TShark, on Windows. <br>It is supported, for at least some interfaces, on some versions of Linux, FreeBSD, and NetBSD.<br><br>Command to put Wireless card in monitor mode on linux (Use sudo in front, if not in root login)<br>ifconfig wlan0 down<br>iwconfig wlan0 mode monitor <br>ifconfig wlan0 up
<div style="clear: both;"></div>
</div>
