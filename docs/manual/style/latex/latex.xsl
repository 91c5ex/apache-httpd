<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0"
              xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output 
  method="text"
  encoding="ISO-8859-1"
  indent="no"
/>

<!-- Read the localized messages from the specified language file -->
<xsl:variable name="messages" select="document('../lang/en.xml')/messages"/>

<!-- Get the guts of the stylesheets -->
<xsl:include href="manualpage.xsl" />
<xsl:include href="common.xsl" />
<xsl:include href="html.xsl" />
<xsl:include href="synopsis.xsl" />

<xsl:template match="sitemap">
<xsl:text>
\documentclass[11pt]{book}
\usepackage{fullpage,hyperref}
\usepackage{style/latex/atbeginend}

% Let LaTeX be lenient about very-bad line wrapping.
\tolerance=9999 
\emergencystretch=40pt

% Keep paragraphs flush left (rather than the default of indenting
% the first line) and put a space between paragraphs.
\setlength{\parindent}{0ex}
\addtolength{\parskip}{1.5ex}

% Shrink the inter-item spaces
\AfterBegin{itemize}{\addtolength{\itemsep}{-.8\baselineskip}}

\pagestyle{headings}

\title{</xsl:text>
<xsl:value-of select="$messages/message[@name='apache']" />
<xsl:text> </xsl:text>
<xsl:value-of select="$messages/message[@name='http-server']" />
<xsl:text> </xsl:text>
<xsl:value-of select="$messages/message[@name='documentation']" />
<xsl:text> </xsl:text>
<xsl:value-of select="$messages/message[@name='version']" />
<xsl:text>}
\author{Apache Software Foundation}
\date{\today}

\begin{document}
\maketitle
\tableofcontents
</xsl:text>

<xsl:for-each select="category">
  <xsl:text>\chapter{</xsl:text>
  <xsl:apply-templates select="title" mode="printcat"/>
  <xsl:text>}
</xsl:text>
    <xsl:apply-templates/>
</xsl:for-each>

<xsl:text>\end{document}</xsl:text>
</xsl:template>

<xsl:template match="page">
<xsl:text>\include{</xsl:text>
<xsl:choose>
<xsl:when test="contains(@href,'.')">
  <xsl:value-of select="substring-before(@href,'.')"/>
</xsl:when>
<xsl:otherwise>
  <xsl:value-of select="concat(@href,'index')"/>
</xsl:otherwise>
</xsl:choose>
<xsl:text>}
</xsl:text>
</xsl:template>

<xsl:template match="category/title" mode="printcat">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="category/title"></xsl:template>

<xsl:template match="modulefilelist">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="modulefile">
<xsl:text>\include{mod/</xsl:text>
<xsl:value-of select="substring-before(.,'.')"/>
<xsl:text>}
</xsl:text>
</xsl:template>

<xsl:template match="summary">
<xsl:apply-templates/>
</xsl:template>

<xsl:template name="replace-string">
  <xsl:param name="text"/>
  <xsl:param name="replace"/>
  <xsl:param name="with"/>
    
  <xsl:choose>
    <xsl:when test="not(contains($text,$replace))">
      <xsl:value-of select="$text"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="substring-before($text,$replace)"/>
      <xsl:value-of select="$with"/>
      <xsl:call-template name="replace-string">
        <xsl:with-param name="text" select="substring-after($text,$replace)"/>
        <xsl:with-param name="replace" select="$replace"/>
        <xsl:with-param name="with" select="$with"/>
       </xsl:call-template>
     </xsl:otherwise>
   </xsl:choose>
</xsl:template>

<!-- ==================================================================== -->
<!-- Take care of all the LaTeX special characters.                       -->
<!-- Silly multi-variable technique used to avoid deep recursion.         -->
<!-- ==================================================================== -->
<xsl:template match="text()">
<xsl:call-template name="ltescape">
  <xsl:with-param name="string" select="."/>
</xsl:call-template>
</xsl:template>


<xsl:template name="ltescape">
<xsl:param name="string"/>

<xsl:variable name="result1">
 <xsl:choose>
 <xsl:when test="contains($string, '\')">
   <xsl:call-template name="replace-string">
    <xsl:with-param name="replace" select="'\'"/>
    <xsl:with-param name="with" select="'\textbackslash '"/>
    <xsl:with-param name="text" select="normalize-space($string)"/>
   </xsl:call-template>
 </xsl:when>
 <xsl:otherwise>
   <xsl:value-of select="."/>
 </xsl:otherwise>
 </xsl:choose>
</xsl:variable>

<xsl:variable name="result2">
 <xsl:choose>
 <xsl:when test="contains($result1, '$')">
   <xsl:call-template name="replace-string">
    <xsl:with-param name="replace" select="'$'"/>
    <xsl:with-param name="with" select="'\$'"/>
    <xsl:with-param name="text" select="$result1"/>
   </xsl:call-template>
 </xsl:when>
 <xsl:otherwise>
   <xsl:value-of select="$result1"/>
 </xsl:otherwise>
 </xsl:choose>
</xsl:variable>

<xsl:variable name="result3">
 <xsl:choose>
 <xsl:when test="contains($result2, '{')">
   <xsl:call-template name="replace-string">
    <xsl:with-param name="replace" select="'{'"/>
    <xsl:with-param name="with" select="'\{'"/>
    <xsl:with-param name="text" select="$result2"/>
   </xsl:call-template>
 </xsl:when>
 <xsl:otherwise>
   <xsl:value-of select="$result2"/>
 </xsl:otherwise>
 </xsl:choose>
</xsl:variable>

<xsl:variable name="result4">
 <xsl:choose>
 <xsl:when test="contains($result3, '}')">
   <xsl:call-template name="replace-string">
    <xsl:with-param name="replace" select="'}'"/>
    <xsl:with-param name="with" select="'\}'"/>
    <xsl:with-param name="text" select="$result3"/>
   </xsl:call-template>
 </xsl:when>
 <xsl:otherwise>
   <xsl:value-of select="$result3"/>
 </xsl:otherwise>
 </xsl:choose>
</xsl:variable>

<xsl:variable name="result5">
 <xsl:choose>
 <xsl:when test="contains($result4, '[')">
   <xsl:call-template name="replace-string">
    <xsl:with-param name="replace" select="'['"/>
    <xsl:with-param name="with" select="'{[}'"/>
    <xsl:with-param name="text" select="$result4"/>
   </xsl:call-template>
 </xsl:when>
 <xsl:otherwise>
   <xsl:value-of select="$result4"/>
 </xsl:otherwise>
 </xsl:choose>
</xsl:variable>

<xsl:variable name="result6">
 <xsl:choose>
 <xsl:when test="contains($result5, ']')">
   <xsl:call-template name="replace-string">
    <xsl:with-param name="replace" select="']'"/>
    <xsl:with-param name="with" select="'{]}'"/>
    <xsl:with-param name="text" select="$result5"/>
   </xsl:call-template>
 </xsl:when>
 <xsl:otherwise>
   <xsl:value-of select="$result5"/>
 </xsl:otherwise>
 </xsl:choose>
</xsl:variable>

    <xsl:call-template name="replace-string">
    <xsl:with-param name="replace" select="'_'"/>
    <xsl:with-param name="with" select="'\_'"/>
    <xsl:with-param name="text">
      <xsl:call-template name="replace-string">
      <xsl:with-param name="replace" select="'#'"/>
      <xsl:with-param name="with" select="'\#'"/>
      <xsl:with-param name="text">
        <xsl:call-template name="replace-string">
        <xsl:with-param name="replace" select="'%'"/>
        <xsl:with-param name="with" select="'\%'"/>
        <xsl:with-param name="text">
          <xsl:call-template name="replace-string">
          <xsl:with-param name="replace" select="'&gt;'"/>
          <xsl:with-param name="with" select="'\textgreater{}'"/>
          <xsl:with-param name="text">
            <xsl:call-template name="replace-string">
            <xsl:with-param name="replace" select="'&lt;'"/>
            <xsl:with-param name="with" select="'\textless{}'"/>
            <xsl:with-param name="text">
              <xsl:call-template name="replace-string">
              <xsl:with-param name="replace" select="'~'"/>
              <xsl:with-param name="with" select="'\textasciitilde{}'"/>
              <xsl:with-param name="text">
                <xsl:call-template name="replace-string">
                <xsl:with-param name="replace" select="'^'"/>
                <xsl:with-param name="with" select="'\^{}'"/>
                <xsl:with-param name="text">
                    <xsl:call-template name="replace-string">
                    <xsl:with-param name="replace" select="'&amp;'"/>
                    <xsl:with-param name="with" select="'\&amp;'"/>
                    <xsl:with-param name="text" select="$result6"/>
                    </xsl:call-template>
                </xsl:with-param>
                </xsl:call-template>
              </xsl:with-param>
              </xsl:call-template>
            </xsl:with-param>
            </xsl:call-template>
          </xsl:with-param>
          </xsl:call-template>
        </xsl:with-param>
        </xsl:call-template>
      </xsl:with-param>
      </xsl:call-template>
    </xsl:with-param>
    </xsl:call-template>

</xsl:template>


</xsl:stylesheet>
