<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="text" indent="yes" encoding="UTF-8"/>

  <xsl:template name="newline">
    <xsl:text>&#10;</xsl:text>
  </xsl:template>

  <!-- See https://mantisarchive.slicer.org/view.php?id=2536 -->
  <xsl:template match="name">
    <xsl:text> (*</xsl:text>
    <xsl:value-of select="."/>
    <xsl:text>*)</xsl:text>
  </xsl:template>

  <xsl:template match="/executable">
    <xsl:text>## Panels and their use</xsl:text>
    <xsl:call-template name="newline"/>
    <xsl:call-template name="newline"/>
    <xsl:for-each select="parameters">
      <xsl:text>### </xsl:text>
      <xsl:value-of select="label"/>
      <xsl:apply-templates select="name"/>
      <xsl:text>: </xsl:text>
      <xsl:value-of select="description"/>
      <xsl:call-template name="newline"/>
      <xsl:for-each select="*[descendant::label]">
        <xsl:text>- **</xsl:text>
        <xsl:value-of select="label"/>
        <xsl:text>**</xsl:text>
        <xsl:apply-templates select="name"/>
        <xsl:text>: </xsl:text>
        <xsl:value-of select="description"/>
        <xsl:call-template name="newline"/>
        <xsl:for-each select="*[descendant::label]">
          <xsl:text>    - **</xsl:text>
          <xsl:value-of select="label"/>
          <xsl:text>**</xsl:text>
          <xsl:apply-templates select="name"/>
          <xsl:text>: </xsl:text>
          <xsl:value-of select="description"/>
          <xsl:call-template name="newline"/>
        </xsl:for-each>
      </xsl:for-each>
      <xsl:call-template name="newline"/>
    </xsl:for-each>
    <xsl:text>## Contributors</xsl:text>
    <xsl:call-template name="newline"/>
    <xsl:call-template name="newline"/>
    <xsl:value-of select="contributor"/>
    <xsl:call-template name="newline"/>
    <xsl:call-template name="newline"/>
    <xsl:text>## Acknowledgements</xsl:text>
    <xsl:call-template name="newline"/>
    <xsl:call-template name="newline"/>
    <xsl:value-of select="acknowledgements"/>
    <xsl:call-template name="newline"/>
  </xsl:template>
</xsl:transform>
