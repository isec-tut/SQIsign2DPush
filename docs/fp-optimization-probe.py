import ctypes as C, pathlib, subprocess, random, time, statistics, json
root=pathlib.Path.cwd(); out=pathlib.Path('/tmp/sqisign-fp-analysis')
out.mkdir(parents=True, exist_ok=True)
rng=random.Random(20260908); results=[]
def chain(e,name):
    lines=[f'void {name}(digit_t *out, const digit_t *a) {{','fp_t t[16], a2, r;','fp_copy(t[0], a); fp_sqr(a2,a);']
    lines += [f'fp_mul(t[{j}],t[{j-1}],a2);' for j in range(1,16)]
    i=e.bit_length()-1; first=True; ns=1; nm=15; x=0
    while i>=0:
        if not (e>>i)&1:
            lines+=['fp_sqr(r,r);']; ns+=1; x*=2; i-=1
        else:
            k=max(0,i-4)
            while not (e>>k)&1:k+=1
            v=(e>>k)&((1<<(i-k+1))-1)
            if first:
                lines+=[f'fp_copy(r,t[{v//2}]);']; x=v; first=False
            else:
                lines+=[f'for(int j=0;j<{i-k+1};j++) fp_sqr(r,r);',f'fp_mul(r,r,t[{v//2}]);']; ns+=i-k+1; nm+=1; x=x*2**(i-k+1)+v
            i=k-1
    assert x==e
    return '\n'.join(lines+['fp_copy(out,r);','}']),[ns,nm]
for lvl,n,a,b in [(1,4,131,78),(3,6,191,117),(5,8,263,156)]:
    p=2**a*3**b-1; R=2**(64*n); A=C.c_uint64*n
    exp,ec=chain((p-3)//4,'cand_exp'); sq,sc=chain((p+1)//4,'cand_sqrt')
    inv3=pow(3,-1,p)*R%p
    cs=', '.join(f'UINT64_C(0x{inv3>>(64*i)&(2**64-1):016x})' for i in range(n))
    code='#include <fp.h>\n'+exp+'\n'+sq+f'\nstatic const fp_t inv3={{{cs}}};\nvoid cand_div3(digit_t *d,const digit_t *x) {{fp_mul(d,x,inv3);}}\n'
    code+='''
uint64_t run_batch(int op,int count,const digit_t *input) {
 fp_t x,t; fp_copy(x,input);
 for(int i=0;i<count;i++) {
 switch(op) {
 case 0: fp_exp3div4(t,x); break;
 case 1: cand_exp(t,x); break;
 case 2: fp_copy(t,x); fp_sqrt(t); break;
 case 3: cand_sqrt(t,x); break;
 case 4: fp_div3(t,x); break;
 case 5: cand_div3(t,x); break;
 case 6: fp_mul(t,x,x); break;
 case 7: fp_sqr(t,x); break;
 }
 fp_copy(x,t);
 }
 return x[0];
}
'''
    f=out/f'candidate{lvl}.c'; f.write_text(code)
    args=['cc','-O3','-funroll-loops','-shared','-fPIC','-Wl,-Bsymbolic','-DDISABLE_NAMESPACING','-DRADIX_64','-DTARGET_AMD64','-DTARGET_OS_UNIX','-DSQISIGN_BUILD_TYPE_REF',f'-DSQISIGN_VARIANT=lvl{lvl}']
    for inc in ['include','src/common/generic/include',f'src/precomp/ref/lvl{lvl}/include',f'src/gf/ref/lvl{lvl}/include']:args+=['-I',str(root/inc)]
    args += [str(root/f'src/gf/ref/lvl{lvl}/fp.c'),str(root/f'src/gf/ref/lvl{lvl}/fp_hd{n*64}.c')]
    fixed = root/f'src/gf/ref/lvl{lvl}/fp_exp.c'
    if fixed.exists():
        args.append(str(fixed))
    args += [str(f),'-o',str(out/f'probe{lvl}.so')]
    proc=subprocess.run(args,capture_output=True,text=True)
    if proc.returncode:raise RuntimeError(proc.stderr)
    lib=C.CDLL(str(out/f'probe{lvl}.so'))
    arr=lambda x:A(*[(x>>(64*i))&(2**64-1) for i in range(n)])
    val=lambda x:sum(int(x[i])<<(64*i) for i in range(n))
    samples=[0,1,2,3,p-1]+[rng.randrange(1,p) for _ in range(100)]
    for x in samples:
        xm=arr(x*R%p); d=A(); old=A()
        for fn,orig,e in [('cand_exp','fp_exp3div4',(p-3)//4),('cand_sqrt','fp_sqrt',(p+1)//4),('cand_div3','fp_div3',None)]:
            getattr(lib,fn)(d,xm)
            expected=(pow(x,e,p) if e is not None else x*pow(3,-1,p)%p)*R%p
            assert val(d)==expected,(lvl,fn,x)
            if orig=='fp_sqrt':old=arr(val(xm));lib.fp_sqrt(old)
            else:getattr(lib,orig)(old,xm)
            assert val(old)==expected,(lvl,orig,x)
        ym=arr(rng.randrange(p)); lib.fp_mul(d,xm,ym)
        assert val(d)==val(xm)*val(ym)*pow(R,-1,p)%p
        lib.fp_sqr(d,xm);assert val(d)==val(xm)**2*pow(R,-1,p)%p
    raw=(C.c_ubyte*(8*n)).from_buffer_copy((1).to_bytes(8*n,'little')); decoded=A()
    lib.fp_decode.restype=C.c_uint32;ok=lib.fp_decode(decoded,raw)
    decode_one_ok=(val(decoded)==R%p)
    reduce_in=(C.c_ubyte*(8*n)).from_buffer_copy((R-1).to_bytes(8*n,'little')); reduced=A();lib.fp_decode_reduce(reduced,reduce_in,8*n)
    reduce_max_ok=(val(reduced)==((R-1)%p)*R%p)
    lib.run_batch.restype=C.c_uint64
    tm=[]; inp=arr(7*R%p)
    for op in range(8):
        count=1000 if op<5 else 100000
        lib.run_batch(op,30,inp)
        runs=[]
        for _ in range(5):
            t=time.perf_counter_ns(); lib.run_batch(op,count,inp);runs.append((time.perf_counter_ns()-t)/count)
        tm.append(round(statistics.median(runs),1))
    row=dict(lvl=lvl,p_bits=p.bit_length(),R_over_p=R/p,exp_SM=ec,sqrt_SM=sc,ns=dict(zip(['exp_old','exp_window','sqrt_old','sqrt_window','div3_old','div3_const','mul','sqr'],tm)),decode_one_ok=decode_one_ok,decode_one_result=hex(val(decoded)),decode_one_expected=hex(R%p),decode_reduce_max_ok=reduce_max_ok,THREE_INV=cs)
    print(json.dumps(row),flush=True);results.append(row)
(out/'results.json').write_text(json.dumps(results,indent=2))
