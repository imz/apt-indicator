
HTMLDOCDIR=html/
STYLESHEET=/usr/share/xml/docbook/xsl-stylesheets/html/chunk.xsl

XMLLINT=xmllint --noout --noent --xinclude --postvalid
XSLTPROC=xsltproc --xinclude  --stringparam chunker.output.encoding "utf-8"
         
TRANSLATIONS = ru uk en

all:
	@for i in $(TRANSLATIONS); do mkdir -p $(HTMLDOCDIR)/$$i; \
	$(XSLTPROC) --stringparam base.dir $(HTMLDOCDIR)/$$i/ $(STYLESHEET) $$i/main.xml; done

check:
	@for i in $(TRANSLATIONS);do $(XMLLINT) $$i/main.xml; done

install:
	@ln -sf ../doc/$(NAME)-$(VERSION)/html $(DATADIR)/doc

clean:
	rm -rf $(HTMLDOCDIR)
