#!/bin/bash
# Begin /usr/sbin/make-ca.sh
#
# Script to create OpenSSL certs directory, GnuTLS certificate bundle
# from upstream certdata.txt.
#
# Authors: DJ Lucas
#          Bruce Dubbs
#
# Original script available at [1] and adapted by Jean-Christophe Fillion-Robin
# to only generate a .crt bundle needed for OpenSSL.
#
# [1] http://anduin.linuxfromscratch.org/BLFS/other/make-ca.sh-20170514
#
# Changes:
#
# 20180305 - Simplify the script keeping minimum functionality allowing to generate
#            .crt bundle:
#            * Remove support for WITH_NSS and WITH_JAVA
#            * Remove support for --pkidir, --nssdb, --keytool, --javacerts, --anchordir, --certutil, --cafile,
#                                 --openssl, --force, --cadir, --localdir  and --ssldir.
#            * Remove "Get p11-kit label, oid, and values"
#            * Remove "Place certificate into trust anchors dir"
#            * Import:
#              -> Keep only the import of certificates trusted for SSL/TLS into GnuTLS certificate bundle
#              -> Remove import into the Java keystore
#            * Remove install rules (Java Cacerts, NSS Shared DB, etc ...)
#
# 20170425 - Use p11-kit format anchors
#          - Add CKA_NSS_MOZILLA_CA_POLICY attribute for p11-kit anchors
#          - Add clientAuth OpenSSL attribute and (currently unused) NSS
#            CKA_TRUST_CLIENT_AUTH
# 20170119 - Show trust bits on local certs
#          - Add version output for help2man
# 20161210 - Add note about --force switch when same version
# 20161126 - Add -D/--destdir switch
# 20161124 - Add -f/--force switch to bypass version check
#          - Add multiple switches to allow for alternate localtions
#          - Add help text
# 20161118 - Drop make-cert.pl script
#          - Add support for Java and NSSDB

# Set defaults
VERSION="20170425"
CERTDATA="certdata.txt"
OPENSSL="/usr/bin/openssl"
DESTDIR="."

# Some data in the certs have UTF-8 characters
export LANG=en_US.utf8

TEMPDIR=$(mktemp -d)
WORKDIR="${TEMPDIR}/work"

function get_args(){
  while test -n "${1}" ; do
    case "${1}" in
      -C | --certdata)
        check_arg $1 $2
        CERTDATA="${2}"
        shift 2
      ;;
      -D | --destdir)
        check_arg $1 $2
        DESTDIR="${2}"
        shift 2
      ;;
      -h | --help)
        showhelp
        exit 0
      ;;
      -v | --version)
        echo -e "$(basename ${0}) ${VERSION}\n"
        exit 0
      ;;
      *)
        showhelp
        exit 1
      ;;
    esac
  done
}

function check_arg(){
  echo "${2}" | grep -v "^-" > /dev/null
  if [ -z "$?" -o ! -n "$2" ]; then
    echo "Error:  $1 requires a valid argument."
    exit 1
  fi
}

function showhelp(){
  echo ""
  echo "`basename ${0}` converts certdata.txt (provided by the Mozilla Foundation)"
  echo "into a complete PKI distribution for use with LFS or like distributions."
  echo ""
  echo "        -C  --certdata   The certdata.txt file (provided by Mozilla)"
  echo "                         Default: ./certdata.txt"
  echo ""
  echo "        -D  --destdir    Change the output directory and use relative"
  echo "                         paths for all other values."
  echo "                         Default: unset"
  echo ""
  echo "        -h  --help       Show this help message and exit"
  echo ""
  echo "        -v  --version    Show version information and exit"
  echo ""
  echo "Example: `basename ${0}` -f -C ~/certdata.txt"
  echo ""
}

# Convert CKA_TRUST values to trust flags
function convert_trust(){
  case $1 in
    CKT_NSS_TRUSTED_DELEGATOR)
      echo "C"
    ;;
    CKT_NSS_NOT_TRUSTED)
      echo "p"
    ;;
    CKT_NSS_MUST_VERIFY_TRUST)
      echo ""
    ;;
  esac
}

function convert_trust_arg(){
  case $1 in
    C)
      case $2 in
        sa)
          echo "-addtrust serverAuth"
        ;;
        sm)
          echo "-addtrust emailProtection"
        ;;
        cs)
          echo "-addtrust codeSigning"
        ;;
        ca)
          echo "-addtrust clientAuth"
        ;;
      esac
    ;;
    p)
      case $2 in
        sa)
          echo "-addreject serverAuth"
        ;;
        sm)
          echo "-addreject emailProtection"
        ;;
        cs)
          echo "-addreject codeSigning"
        ;;
        ca)
          echo "-addreject clientAuth"
        ;;
      esac
    ;;
    *)
      echo ""
    ;;
  esac
}

# Define p11-kit ext value constants (see p11-kit API documentation)
get-p11-val() {
  case $1 in
    p11sasmcs)
      p11value="0%2a%06%03U%1d%25%01%01%ff%04 0%1e%06%08%2b%06%01%05%05%07%03%04%06%08%2b%06%01%05%05%07%03%01%06%08%2b%06%01%05%05%07%03%03"
    ;;

    p11sasm)
      p11value="0 %06%03U%1d%25%01%01%ff%04%160%14%06%08%2b%06%01%05%05%07%03%04%06%08%2b%06%01%05%05%07%03%01"
    ;;

    p11sacs)
      p11value="0 %06%03U%1d%25%01%01%ff%04%160%14%06%08%2b%06%01%05%05%07%03%01%06%08%2b%06%01%05%05%07%03%03"
    ;;

    p11sa)
      p11value="0%16%06%03U%1d%25%01%01%ff%04%0c0%0a%06%08%2b%06%01%05%05%07%03%01"
    ;;

    p11smcs)
      p11value="0 %06%03U%1d%25%01%01%ff%04%160%14%06%08%2b%06%01%05%05%07%03%04%06%08%2b%06%01%05%05%07%03%03"
    ;;

    p11sm)
      p11value="0%16%06%03U%1d%25%01%01%ff%04%0c0%0a%06%08%2b%06%01%05%05%07%03%04"
    ;;

    p11cs)
      p11value="0%16%06%03U%1d%25%01%01%ff%04%0c0%0a%06%08%2b%06%01%05%05%07%03%03"
    ;;

    p11)
      p11value="0%18%06%03U%1d%25%01%01%ff%04%0e0%0c%06%0a%2b%06%01%04%01%99w%06%0a%10"
    ;;
  esac
}

# Process command line arguments
get_args $@

if test ! -r "${CERTDATA}"; then
  echo "${CERTDATA} was not found. The certdata.txt file must be in the local"
  echo "directory, or specified with the --certdata switch."
  exit 1
fi

mkdir -p "${TEMPDIR}"/{certs,ssl/certs,work}
cp "${CERTDATA}" "${WORKDIR}/certdata.txt"
pushd "${WORKDIR}" > /dev/null

# Get a list of starting lines for each cert
CERTBEGINLIST=`grep -n "^# Certificate" "${WORKDIR}/certdata.txt" | \
                      cut -d ":" -f1`

# Dump individual certs to temp file
for certbegin in ${CERTBEGINLIST}; do
  awk "NR==$certbegin,/^CKA_TRUST_STEP_UP_APPROVED/" "${WORKDIR}/certdata.txt" \
      > "${TEMPDIR}/certs/${certbegin}.tmp"
done

unset CERTBEGINLIST certbegin

for tempfile in ${TEMPDIR}/certs/*.tmp; do
  # Get a name for the cert
  certname="$(grep "^# Certificate" "${tempfile}" | cut -d '"' -f 2)"

  # Determine certificate trust values for SSL/TLS, S/MIME, and Code Signing
  satrust="$(convert_trust `grep '^CKA_TRUST_SERVER_AUTH' ${tempfile} | \
                  cut -d " " -f 3`)"
  smtrust="$(convert_trust `grep '^CKA_TRUST_EMAIL_PROTECTION' ${tempfile} | \
                  cut -d " " -f 3`)"
  cstrust="$(convert_trust `grep '^CKA_TRUST_CODE_SIGNING' ${tempfile} | \
                  cut -d " " -f 3`)"
  # Not currently included in NSS certdata.txt
  #catrust="$(convert_trust `grep '^CKA_TRUST_CLIENT_AUTH' ${tempfile} | \
  #                cut -d " " -f 3`)"

  # Get args for OpenSSL trust settings
  saarg="$(convert_trust_arg "${satrust}" sa)"
  smarg="$(convert_trust_arg "${smtrust}" sm)"
  csarg="$(convert_trust_arg "${cstrust}" cs)"
  # Not currently included in NSS certdata.txt
  #caarg="$(convert_trust_arg "${catrust}" ca)"

  # Convert to a PEM formatted certificate
  printf $(awk '/^CKA_VALUE/{flag=1;next}/^END/{flag=0}flag{printf $0}' \
  "${tempfile}") | "${OPENSSL}" x509 -text -inform DER -fingerprint \
  > tempfile.crt

  # Get individual values for certificates
  certkey="$(${OPENSSL} x509 -in tempfile.crt -noout -pubkey)"
  certcer="$(${OPENSSL} x509 -in tempfile.crt)"
  certtxt="$(${OPENSSL} x509 -in tempfile.crt -noout -text)"

  # if distrusted at all, x-distrusted
  if test "${satrust}" == "p" -o "${smtrust}" == "p" -o "${cstrust}" == "p"
  then
      # if any distrusted, x-distrusted
      p11trust="x-distrusted: true"
      p11oid="1.3.6.1.4.1.3319.6.10.1"
      p11value="0.%06%0a%2b%06%01%04%01%99w%06%0a%01%04 0%1e%06%08%2b%06%01%05%05%07%03%04%06%08%2b%06%01%05%05%07%03%01%06%08%2b%06%01%05%05%07%03%03"
  else
      p11trust="trusted: true"
      p11oid="2.5.29.37"
      trustp11="p11"
      if test "${satrust}" == "C"; then
          trustp11="${trustp11}sa"
      fi
      if test "${smtrust}" == "C"; then
          trustp11="${trustp11}sm"
      fi
      if test "${cstrust}" == "C"; then
          trustp11="${trustp11}cs"
      fi
      get-p11-val "${trustp11}"
  fi

  # Get a hash for the cert
  keyhash=$("${OPENSSL}" x509 -noout -in tempfile.crt -hash)

  # Print information about cert
  echo "Certificate:  ${certname}"
  echo "Keyhash:      ${keyhash}"

  # Import certificates trusted for SSL/TLS into
  # GnuTLS certificate bundle
  if test "${satrust}x" == "Cx"; then
    # GnuTLS certificate bundle
    cat tempfile.crt >> "${TEMPDIR}/ssl/ca-bundle.crt"
    echo "Added to GnuTLS ceritificate bundle."
  fi

  # Import certificate into the temporary certificate directory with
  # trust arguments
  "${OPENSSL}" x509 -in tempfile.crt -text -fingerprint \
      -setalias "${certname}" ${saarg} ${smarg} ${csarg}    \
      > "${TEMPDIR}/ssl/certs/${keyhash}.pem"
  #echo "Added to OpenSSL certificate directory with trust '${satrust},${smtrust},${cstrust}'."

  # Clean up the directory and environment as we go
  rm -f tempfile.crt
  unset keyhash subject certname
  unset satrust smtrust cstrust catrust sarg smarg csarg caarg
  unset p11trust p11oid p11value trustp11 certkey certcer certtxt
  echo -e "\n"
done
unset tempfile

# Sanity check
count=$(ls "${TEMPDIR}"/ssl/certs/*.pem | wc -l)
# Historically there have been between 152 and 165 certs
# A minimum of 140 should be safe for a rudimentry sanity check
if test "${count}" -lt "140" ; then
    echo "Error! Only ${count} certificates were generated!"
    echo "Exiting without update!"
    echo ""
    echo "${TEMPDIR} is the temporary working directory"
    exit 2
fi
unset count

popd > /dev/null

# Copy the generated bundle
echo "Copy ${TEMPDIR}/ssl/ca-bundle.crt to ${DESTDIR}/Slicer.crt"
cp "${TEMPDIR}/ssl/ca-bundle.crt" "${DESTDIR}/Slicer.crt"

# Clean up the mess
rm -rf "${TEMPDIR}"

# End /usr/sbin/make-ca.sh

