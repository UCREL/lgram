
d = read.table("TIME", header=T)

#par(cex.lab=20)
#par(cex.axis=20)

pdf("memory.pdf", width=5, height=5)
plot(d$NSP[d$type=='m'], ylim=c(0,2000),type='l', col=2, lwd=3, ylab="memory (mb)", xaxt='n', xlab="words (1k)", main="memory")
axis(1, 1:6, d$words[d$type=='m'])
lines(d$lgram[d$type=='m'], col=3, lwd=3)
lines(d$kfNgram[d$type=='m'], col=4, lwd=3)
legend(2000, 160, c("NSP", "lgram", "kfNgram") )
legend(1, 2000, c("NSP", "lgram", "kfNgram"), col=c(2:4), lwd=3)
dev.off()


pdf("time.pdf", width=5, height=5)
plot(d$lgram[d$type=='t'], ylim=c(0,2000),type='l', col=3, lwd=3, ylab="time (seconds)", xaxt='n', xlab="words (1k)", main="time")
axis(1, 1:6, d$words[d$type=='t'])
lines(d$NSP[d$type=='t'], col=2, lwd=3)
lines(d$kfNgram[d$type=='t'], col=4, lwd=3)
legend(1, 2000, c("NSP", "lgram", "kfNgram"), col=c(2:4), lwd=3)
dev.off()


