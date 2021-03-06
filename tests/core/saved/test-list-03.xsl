<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" version="1.0">
  <xsl:strip-space elements="itemlist"/>
  <xsl:template match="list">
    <list>
      <xsl:apply-templates/>
    </list>
  </xsl:template>
  <xsl:template match="item">
    <fo:list-item indent-start="2pi">
      <fo:list-item-label>
        <xsl:variable name="level" select="count(ancestor::orderedlist) mod 3"/>
        <xsl:choose>
          <xsl:when test="$level = 1">
            <number format="i"/>
          </xsl:when>
          <xsl:when test="$level = 2">
            <number format="a"/>
          </xsl:when>
          <xsl:otherwise>
            <number format="1"/>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:text>. </xsl:text>
      </fo:list-item-label>
      <fo:list-item-body>
        <xsl:apply-templates/>
      </fo:list-item-body>
    </fo:list-item>
  </xsl:template>
</xsl:stylesheet>
