# Simulation-and-Prevention-of-Low-Rate-DDoS-attacks
This repo contains the course project of Communication Networks (EE706).\
**Problem Description** \
Denial of Service attacks can prevent or degrade
the service of users by consuming resources in networks, server
clusters, or end hosts. Many different varieties of DoS attacks
exist, such as TCP SYN attacks or DNS flood attacks, but one
commonality is that these attacks generally require high-rate
transmission of packets. Here, we simulate a ’Low-Rate TCP tar-
getted DoS attack’ which is harder to detect as compared to the
traditional DoS attacks. These DoS attacks make use of the TCP
congestion control’s retransmission timeout (RTO) functionality
to stop communication between a sender and a receiver. In later
section, we explore different prevention techniques and found
based on previous researches that the existing techniques does
not perform well or have some negative side effects. For these
stated reasons we also provide ML based prevention techniques
that gives upto 98.23%. Such ML based models not only prevent
from LDOS attack but can also be used to prevent from different
types of attack on network.
\
\
**Dataset used:**
I trained and evaluated my model on ISCX2012 intrusion detection evaluation
dataset. \ [link to dataset](https://www.unb.ca/cic/datasets/ids.html)
\
\
**Algorithms used**
- LSTM
- GRU
- RNN
\
\
**References:**
1. A. Kuzmanovic and E. W. Knightly, “Low-rate tcp-targeted denial of
service attacks: the shrew vs. the mice and elephants,” in Proceedings
of the 2003 conference on Applications, technologies, architectures, and
protocols for computer communications, 2003, pp. 75–86.
\
2. W. Zhijun, L. Wenjing, L. Liang, and Y. Meng, “Low-rate dos attacks,
detection, defense, and challenges: A survey,” IEEE Access, vol. 8, pp.
43 920–43 943, 2020.
