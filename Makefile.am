# filepath: /home/nico/GITHUB_REPOS/tcp_congestion_linux_cmp/Makefile.am
bin_PROGRAMS = tcp_comparison
tcp_comparison_SOURCES = src/tcp_comparison_linux_common_policies.cpp
tcp_comparison_CPPFLAGS = -I$(srcdir)/src/bcc_minimal/include -DHAVE_BCC -DENABLE_EBPF_METRICS
AUTOMAKE_OPTIONS = subdir-objects
