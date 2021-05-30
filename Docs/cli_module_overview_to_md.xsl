<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="text" indent="yes" encoding="UTF-8"/>

  <xsl:template name="newline">
    <xsl:text>&#10;</xsl:text>
  </xsl:template>

  <xsl:template match="/executable">
    <xsl:text># </xsl:text>
    <xsl:value-of select="title"/>
    <xsl:call-template name="newline"/>
    <xsl:call-template name="newline"/>
    <xsl:text>## Overview</xsl:text>
    <xsl:call-template name="newline"/>
    <xsl:call-template name="newline"/>
    <xsl:value-of select="description"/>
    <xsl:call-template name="newline"/>
  </xsl:template>
</xsl:transform>
