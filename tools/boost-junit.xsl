<?xml version = "1.0"?> 
<xsl:stylesheet version = "1.0" 
    xmlns:xsl = "http://www.w3.org/1999/XSL/Transform"> 

    <xsl:template match = "/TestResult"> 
        <xsl:apply-templates select = "*"/> 
    </xsl:template> 

    <xsl:template match = "/TestResult/TestSuite"> 
        <xsl:element name = "testsuite"> 
            <xsl:attribute name = "name"> 
                <xsl:value-of select = "@name"/> 
            </xsl:attribute> 
            <xsl:attribute name = "failures"> 
                <xsl:value-of select = "@test_cases_failed"/> 
            </xsl:attribute> 
            <xsl:attribute name = "tests"> 
                <xsl:value-of select = "sum (@test_cases_failed | @test_cases_passed)"/> 
            </xsl:attribute> 
            <xsl:apply-templates select = "*"/> 
        </xsl:element> 
    </xsl:template> 

    <xsl:template match = "TestCase"> 
        <xsl:element name = "testcase"> 
            <xsl:attribute name = "name"> 
                <xsl:value-of select = "@name"/> 
            </xsl:attribute> 
            <xsl:attribute name = "status"> 
                <xsl:value-of select = "@result"/> 
            </xsl:attribute> 
        </xsl:element> 
    </xsl:template> 
</xsl:stylesheet> 
