
TEST_CFLAGS = -Wall -I$(top_srcdir)/include $(liblo_CFLAGS)

if WINDOWS_DLL
TEST_LDADD = $(top_builddir)/src/*.lo $(liblo_LIBS)
else
TEST_LDADD = $(top_builddir)/src/libmapper-@MAJOR_VERSION@.la $(liblo_LIBS)
endif

noinst_PROGRAMS = test testmany testadmin testsend testrecv			\
                  testexpression testlinear testparams testmonitor	\
                  testdb testparser testprops testquery testvector	\
                  testdynamic

testadmin_CFLAGS = $(TEST_CFLAGS)
testadmin_SOURCES = testadmin.c
testadmin_LDADD = $(TEST_LDADD)

testsend_CFLAGS = $(TEST_CFLAGS)
testsend_SOURCES = testsend.c
testsend_LDADD = $(TEST_LDADD)

testrecv_CFLAGS = $(TEST_CFLAGS)
testrecv_SOURCES = testrecv.c
testrecv_LDADD = $(TEST_LDADD)

testlinear_CFLAGS = $(TEST_CFLAGS)
testlinear_SOURCES = testlinear.c
testlinear_LDADD = $(TEST_LDADD)

testvector_CFLAGS = $(TEST_CFLAGS)
testvector_SOURCES = testvector.c
testvector_LDADD = $(TEST_LDADD)

testexpression_CFLAGS = $(TEST_CFLAGS)
testexpression_SOURCES = testexpression.c
testexpression_LDADD = $(TEST_LDADD)

testparams_CFLAGS = $(TEST_CFLAGS)
testparams_SOURCES = testparams.c
testparams_LDADD = $(TEST_LDADD)

testmonitor_CFLAGS = $(TEST_CFLAGS)
testmonitor_SOURCES = testmonitor.c
testmonitor_LDADD = $(TEST_LDADD)

testdb_CFLAGS = $(TEST_CFLAGS)
testdb_SOURCES = testdb.c
testdb_LDADD = $(TEST_LDADD)

testparser_CFLAGS = $(TEST_CFLAGS)
testparser_SOURCES = testparser.c
testparser_LDADD = $(TEST_LDADD)

testprops_CFLAGS = $(TEST_CFLAGS)
testprops_SOURCES = testprops.c
testprops_LDADD = $(TEST_LDADD)

testmany_CFLAGS = $(TEST_CFLAGS)
testmany_SOURCES = testmany.c
testmany_LDADD = $(TEST_LDADD)

test_CFLAGS = $(TEST_CFLAGS)
test_SOURCES = test.c
test_LDADD = $(TEST_LDADD)

testquery_CFLAGS = $(TEST_CFLAGS)
testquery_SOURCES = testquery.c
testquery_LDADD = $(TEST_LDADD)

testdynamic_CFLAGS = $(TEST_CFLAGS)
testdynamic_SOURCES = testdynamic.c
testdynamic_LDADD = $(TEST_LDADD)
